// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "ThreatCalculator.h"
#include "AdaptiveConfig.h"
#include "AdaptiveMemory.h"
#include "DynamicWeightManager.h"
#include "Utils/MathUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogThreatCalculator, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	/** Running mean and variance over a fixed-size ring buffer (Welford-style). */
	void ComputeMeanAndVariance(const TArray<float>& Buffer, float& OutMean, float& OutVariance)
	{
		OutMean = 0.f;
		OutVariance = 0.f;
		if (Buffer.IsEmpty()) return;

		for (float V : Buffer) OutMean += V;
		OutMean /= Buffer.Num();

		for (float V : Buffer) OutVariance += FMath::Square(V - OutMean);
		OutVariance /= Buffer.Num();
	}

	void PushToRingBuffer(TArray<float>& Buffer, float Value, int32 MaxSize)
	{
		if (Buffer.Num() >= MaxSize) Buffer.RemoveAt(0);
		Buffer.Add(Value);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Layer 2 main pipeline
// ─────────────────────────────────────────────────────────────────────────────

FThreatAssessment UThreatCalculator::CalculateThreat(
	const FContextData&          Context,
	const UAdaptiveMemory*       Memory,
	const UDynamicWeightManager* WeightsMgr,
	const UAdaptiveConfig*       Config)
{
	FThreatAssessment Result;
	if (!Config) { LastAssessment = Result; return Result; }

	const TArray<float>& W = WeightsMgr ? WeightsMgr->GetWeights() : DefaultWeights;

	// ── Phase 3-4: T_base + T_cross ─────────────────────────────────────────
	Result.ThreatBase  = ComputeTBase(Context, W);
	Result.ThreatCross = ComputeTCross(Context, W, Config);
	Result.ThreatRaw   = FMath::Clamp(Result.ThreatBase + Config->CrossTermBeta * Result.ThreatCross, 0.f, 1.f);

	// ── Phase 5a: Store T_raw for σ² computation ─────────────────────────────
	PushToRingBuffer(TRawHistory, Result.ThreatRaw, Config->ConfidenceWindow);

	// ── Phase 5b: Confidence C = 1 / (1 + σ²) ───────────────────────────────
	Result.Confidence = ComputeConfidence();

	// ── Phase 5c: Exponential smoothing T_smooth = α*T_raw + (1-α)*T_prev ───
	Result.ThreatSmoothed = Config->SmoothingAlpha * Result.ThreatRaw
	                      + (1.f - Config->SmoothingAlpha) * LastTSmooth;
	LastTSmooth = Result.ThreatSmoothed;

	// ── Phase 5d: T_final = C*T_smooth + (1-C)*T_default ────────────────────
	Result.ThreatFinal = FMath::Clamp(
		Result.Confidence * Result.ThreatSmoothed + (1.f - Result.Confidence) * Config->DefaultThreat,
		0.f, 1.f);

	// ── Stage 6K: Adaptive thresholds — rolling μ/σ over T_final ────────────
	PushToRingBuffer(TFinalHistory, Result.ThreatFinal, Config->AdaptiveThresholdWindow);
	float Mean, Variance;
	ComputeMeanAndVariance(TFinalHistory, Mean, Variance);
	Result.AdaptiveMeanThreat   = Mean;
	Result.AdaptiveStdDevThreat = FMath::Sqrt(Variance);

	UE_LOG(LogThreatCalculator, Verbose,
		TEXT("[Threat] T_base=%.3f T_cross=%.3f T_raw=%.3f C=%.3f T_smooth=%.3f T_final=%.3f | μ=%.3f σ=%.3f"),
		Result.ThreatBase, Result.ThreatCross, Result.ThreatRaw,
		Result.Confidence, Result.ThreatSmoothed, Result.ThreatFinal,
		Result.AdaptiveMeanThreat, Result.AdaptiveStdDevThreat);

	LastAssessment = Result;
	return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// T_base = Σ w_i * f_i(x_i)
// Indices: 0=Distance, 1=WeaponThreat, 2=Health, 3=Allies, 4=RoomDensity, 5=Memory
// ─────────────────────────────────────────────────────────────────────────────
float UThreatCalculator::ComputeTBase(const FContextData& Ctx, const TArray<float>& W) const
{
	if (W.Num() < 6) return 0.f;

	// f_D(x): Distance is already sigmoid-transformed by ContextEvaluator (Layer 1)
	const float fD = Ctx.DistanceNorm;

	// f_W(x): linear (weapon type is discrete)
	const float fW = Ctx.WeaponThreatNorm;

	// f_H(x): already x^α from ContextEvaluator (quadratic)
	const float fH = Ctx.EnemyHPRatioNorm;

	// f_A(x) = 1 - sqrt(A_norm): inverse — more allies → less threat
	const float fA = 1.f - FMath::Sqrt(FMath::Clamp(Ctx.AllyCountNorm, 0.f, 1.f));

	// f_R(x): linear
	const float fR = Ctx.RoomDensityNorm;

	// f_M(x): decayed aggressiveness (already computed by ContextEvaluator with Memory)
	// Clamped to [0,1] for formula consistency
	const float fM = FMath::Clamp(Ctx.MemoryAggressiveness, 0.f, 1.f);

	return W[0]*fD + W[1]*fW + W[2]*fH + W[3]*fA + W[4]*fR + W[5]*fM;
}

// ─────────────────────────────────────────────────────────────────────────────
// T_cross = w_DH * f_D * f_H  +  w_WM * W_norm * M_attack
// ─────────────────────────────────────────────────────────────────────────────
float UThreatCalculator::ComputeTCross(const FContextData& Ctx, const TArray<float>& W, const UAdaptiveConfig* Cfg) const
{
	if (!Cfg) return 0.f;

	// Cross-term 1: close + low HP → exponential spike (not just arithmetic addition)
	const float CrossDH = Cfg->CrossWeightDistanceHealth * Ctx.DistanceNorm * Ctx.EnemyHPRatioNorm;

	// Cross-term 2: ranged weapon + player aggression memory
	const float MAttack = FMath::Clamp(Ctx.MemoryAggressiveness, 0.f, 1.f);
	const float CrossWM = Cfg->CrossWeightWeaponMemory * Ctx.WeaponThreatNorm * MAttack;

	return CrossDH + CrossWM;
}

// ─────────────────────────────────────────────────────────────────────────────
// Confidence C = 1 / (1 + σ²)   where σ² = variance(TRawHistory)
// ─────────────────────────────────────────────────────────────────────────────
float UThreatCalculator::ComputeConfidence() const
{
	if (TRawHistory.IsEmpty()) return 1.f;

	float Mean, Variance;
	ComputeMeanAndVariance(TRawHistory, Mean, Variance);
	return 1.f / (1.f + Variance);
}

// ─────────────────────────────────────────────────────────────────────────────
// Adaptive thresholds (Stage 6K)
// ─────────────────────────────────────────────────────────────────────────────
float UThreatCalculator::GetHighThreatThreshold() const
{
	return FMath::Clamp(LastAssessment.AdaptiveMeanThreat + LastAssessment.AdaptiveStdDevThreat, 0.f, 1.f);
}

float UThreatCalculator::GetLowThreatThreshold() const
{
	return FMath::Clamp(LastAssessment.AdaptiveMeanThreat - LastAssessment.AdaptiveStdDevThreat, 0.f, 1.f);
}

