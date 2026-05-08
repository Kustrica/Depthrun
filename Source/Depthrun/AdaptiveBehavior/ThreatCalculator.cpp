// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "ThreatCalculator.h"
#include "AdaptiveConfig.h"
#include "DynamicWeightManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogThreatCalculator, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// Layer 2 main pipeline  (Variant E — simplified)
// T_final = clamp( Σ w_i * f_i(x_i), 0, 1 )
// ─────────────────────────────────────────────────────────────────────────────

FThreatAssessment UThreatCalculator::CalculateThreat(
	const FContextData&          Context,
	const UDynamicWeightManager* WeightsMgr,
	const UAdaptiveConfig*       Config)
{
	FThreatAssessment Result;
	if (!Config) { LastAssessment = Result; return Result; }

	const TArray<float>& W = WeightsMgr ? WeightsMgr->GetWeights() : DefaultWeights;

	// T_base = Σ w_i * f_i(x_i)
	Result.ThreatBase = ComputeTBase(Context, W);

	// No cross-terms, no smoothing — T_raw = T_base
	Result.ThreatRaw = Result.ThreatBase;

	// T_final = clamp(T_raw, 0, 1)
	// Note: when DistanceNorm = 0 (player beyond MaxEngagementRange), fD = 0 and
	// its w_i contribution disappears naturally. Other factors (HP, Memory, etc.)
	// remain active — do NOT force T_final = 0 here.
	Result.ThreatFinal    = FMath::Clamp(Result.ThreatRaw, 0.f, 1.f);
	Result.ThreatSmoothed = Result.ThreatFinal; // kept for debug widget compat
	Result.Confidence     = 1.f;               // kept for debug widget compat

	UE_LOG(LogThreatCalculator, Verbose,
		TEXT("[Threat] T_base=%.3f T_final=%.3f"),
		Result.ThreatBase, Result.ThreatFinal);

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

	// f_D: linear normalization (no sigmoid — simpler, still [0,1])
	const float fD = Ctx.DistanceNorm;

	// f_W: discrete weapon threat lookup [0,1]
	const float fW = Ctx.WeaponThreatNorm;

	// f_H: quadratic inverse HP — x^2 where x = 1 - HPRatio (low HP → high threat)
	const float fH = Ctx.EnemyHPRatioNorm;

	// f_A: inverse — more allies → less threat
	const float fA = 1.f - FMath::Sqrt(FMath::Clamp(Ctx.AllyCountNorm, 0.f, 1.f));

	// f_R: room density [0,1]
	const float fR = Ctx.RoomDensityNorm;

	// f_M: simple aggressiveness counter, normalized to [0,1] by ContextEvaluator
	const float fM = FMath::Clamp(Ctx.MemoryAggressiveness, 0.f, 1.f);

	return W[0]*fD + W[1]*fW + W[2]*fH + W[3]*fA + W[4]*fR + W[5]*fM;
}

