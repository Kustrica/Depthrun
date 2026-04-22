// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DynamicWeightManager.h"
#include "AdaptiveConfig.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicWeights, Log, All);

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
	if (!Config || Weights.Num() < 6) return;

	// Compute raw factor values f_i(x_i) — same transforms as in ComputeTBase
	// Index: 0=Distance, 1=WeaponThreat, 2=Health, 3=Allies, 4=RoomDensity, 5=Memory
	const float FactorValues[6] =
	{
		Context.DistanceNorm,                                                              // f_D
		Context.WeaponThreatNorm,                                                          // f_W
		Context.EnemyHPRatioNorm,                                                          // f_H
		1.f - FMath::Sqrt(FMath::Clamp(Context.AllyCountNorm, 0.f, 1.f)),                 // f_A (inverse)
		Context.RoomDensityNorm,                                                           // f_R
		FMath::Clamp(Context.MemoryAggressiveness, 0.f, 1.f),                             // f_M
	};

	// Σ f_j(x_j) — normalisation denominator
	float SumFactors = 0.f;
	for (int32 i = 0; i < 6; ++i) SumFactors += FactorValues[i];

	// Avoid division by zero when all factors are 0 (e.g. very first tick)
	if (SumFactors < KINDA_SMALL_NUMBER) return;

	// w_i(t+1) = clamp(w_i(t) + η * reward * contribution_i, w_min, w_max)
	// contribution_i = f_i(x_i) / Σ f_j(x_j)
	const float Eta    = Config->WeightLearningRate;
	const float WMin   = Config->WeightMin;
	const float WMax   = Config->WeightMax;

	for (int32 i = 0; i < 6; ++i)
	{
		const float Contribution = FactorValues[i] / SumFactors;
		Weights[i] = FMath::Clamp(Weights[i] + Eta * Reward * Contribution, WMin, WMax);
	}

	UE_LOG(LogDynamicWeights, Verbose,
		TEXT("[Weights] reward=%.1f | w0=%.3f w1=%.3f w2=%.3f w3=%.3f w4=%.3f w5=%.3f"),
		Reward, Weights[0], Weights[1], Weights[2], Weights[3], Weights[4], Weights[5]);
}

