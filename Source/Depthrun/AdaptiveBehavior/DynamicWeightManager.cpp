// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DynamicWeightManager.h"
#include "AdaptiveConfig.h"

void UDynamicWeightManager::ResetToDefaults(const UAdaptiveConfig* Config)
{
	Weights.SetNum(6);
	if (!Config) { Weights.Init(1.f / 6.f, 6); return; }

	Weights[0] = Config->WeightDistance;
	Weights[1] = Config->WeightWeaponThreat;
	Weights[2] = Config->WeightHealth;
	Weights[3] = Config->WeightAllies;
	Weights[4] = Config->WeightRoomDensity;
	Weights[5] = Config->WeightMemory;
}

void UDynamicWeightManager::UpdateWeights(float Reward, const FContextData& Context, const UAdaptiveConfig* Config)
{
	// TODO (Stage 6G): compute contribution_i, apply update rule, clamp to [w_min, w_max]
}
