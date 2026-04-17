// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "DynamicWeightManager.generated.h"

class UAdaptiveConfig;

/**
 * UDynamicWeightManager
 * Maintains and updates the 6 weights w1..w6 used by UThreatCalculator.
 *
 * Update rule (Online, per combat event):
 *   contribution_i = f_i(x_i) / Σ f_j(x_j)
 *   w_i(t+1) = clamp(w_i(t) + η * reward * contribution_i, w_min, w_max)
 *
 *   reward = +1.0 (enemy dealt damage) | -1.0 (enemy took damage)
 *
 * Weight indices (match AdaptiveConfig fields):
 *   0: Distance, 1: WeaponThreat, 2: Health, 3: Allies, 4: RoomDensity, 5: Memory
 *
 * Implementation: Stage 6G.
 */
UCLASS()
class DEPTHRUN_API UDynamicWeightManager : public UObject
{
	GENERATED_BODY()

public:
	/** Initialise weights from Config defaults. Call once in BeginPlay. */
	void ResetToDefaults(const UAdaptiveConfig* Config);

	/**
	 * Apply one weight update step.
	 * @param Reward   +1 (enemy dealt damage) or -1 (enemy took damage).
	 * @param Context  Last evaluated context (for contribution_i computation).
	 * @param Config   Config asset for η, w_min, w_max.
	 */
	void UpdateWeights(float Reward, const FContextData& Context, const UAdaptiveConfig* Config);

	/** Read-only weights for ThreatCalculator and debug widget. */
	const TArray<float>& GetWeights() const { return Weights; }

private:
	/** Current weights [0..5] corresponding to Distance…Memory. */
	TArray<float> Weights;
};
