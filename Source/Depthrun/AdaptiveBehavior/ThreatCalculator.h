// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "ThreatCalculator.generated.h"

class UAdaptiveConfig;
class UAdaptiveMemory;
class UDynamicWeightManager;

/**
 * UThreatCalculator — Layer 2
 * Computes T_final from normalized context data with confidence scoring.
 *
 * Formula pipeline:
 *   T_base    = Σ w_i * f_i(x_i)
 *   T_cross   = w_DH * f_D * f_H  +  w_WM * W * M_attack
 *   T_raw     = T_base + β * T_cross
 *   σ²        = variance(T_raw over last N evaluations)
 *   C         = 1 / (1 + σ²)
 *   T_smooth  = α * T_raw + (1-α) * T_prev
 *   T_final   = C * T_smooth + (1-C) * T_default
 *
 * Also computes adaptive thresholds (μ ± σ) over K evaluations.
 * Implementation: Stage 6F + Stage 6K.
 */
UCLASS()
class DEPTHRUN_API UThreatCalculator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Main entry point. Returns full FThreatAssessment.
	 * Must be called every evaluation tick with current context.
	 */
	FThreatAssessment CalculateThreat(
		const FContextData&     Context,
		const UAdaptiveMemory*  Memory,
		const UDynamicWeightManager* Weights,
		const UAdaptiveConfig*  Config);

	/** Last computed T_final for debug readouts. */
	float GetLastThreatFinal() const { return LastAssessment.ThreatFinal; }

	/** Last computed Confidence for debug readouts. */
	float GetLastConfidence() const { return LastAssessment.Confidence; }

	/** High-threat adaptive threshold (μ + σ) — used by UtilityCurves. */
	float GetHighThreatThreshold() const;

	/** Low-threat adaptive threshold (μ - σ). */
	float GetLowThreatThreshold() const;

private:
	float ComputeTBase(const FContextData& Ctx, const TArray<float>& Weights) const;
	float ComputeTCross(const FContextData& Ctx, const TArray<float>& Weights, const UAdaptiveConfig* Cfg) const;
	float ComputeConfidence() const;

	/** Ring buffer of recent T_raw values for σ² computation. */
	TArray<float> TRawHistory;

	/** Ring buffer of recent T_final values for adaptive thresholds. */
	TArray<float> TFinalHistory;

	float LastTSmooth = 0.f;
	FThreatAssessment LastAssessment;
};
