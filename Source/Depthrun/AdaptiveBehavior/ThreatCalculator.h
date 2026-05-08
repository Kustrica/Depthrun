// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "ThreatCalculator.generated.h"

class UAdaptiveConfig;
class UDynamicWeightManager;

/**
 * UThreatCalculator — Layer 2
 * Computes T_final from normalized context data.
 *
 * Formula pipeline (Variant E — simplified):
 *   T_base  = Σ w_i * f_i(x_i)   (weighted sum, 6 factors)
 *   T_final = clamp(T_base, 0, 1)
 *
 * Removed: cross-terms, exponential smoothing, confidence (σ²),
 *          DefaultThreat fallback, adaptive threshold window.
 * Retained: normalization, quadratic HP, dynamic weights, bell curves,
 *           N-gram patterns, inertia, transition cost matrix.
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
		const FContextData&          Context,
		const UDynamicWeightManager* Weights,
		const UAdaptiveConfig*       Config);

	/** Last computed T_final for debug readouts. */
	float GetLastThreatFinal() const { return LastAssessment.ThreatFinal; }

private:
	float ComputeTBase(const FContextData& Ctx, const TArray<float>& Weights) const;

	/** Fallback weights used if DynamicWeightManager is null. Must match AdaptiveConfig defaults.
	 *  Sum = 1.0: Distance=0.20, Weapon=0.20, Health=0.20, Allies=0.15, Density=0.10, Memory=0.15 */
	TArray<float> DefaultWeights = { 0.20f, 0.20f, 0.20f, 0.15f, 0.10f, 0.15f };

	FThreatAssessment LastAssessment;
};
