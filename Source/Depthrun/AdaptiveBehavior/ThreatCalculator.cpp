// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "ThreatCalculator.h"
#include "AdaptiveConfig.h"
#include "AdaptiveMemory.h"
#include "DynamicWeightManager.h"

FThreatAssessment UThreatCalculator::CalculateThreat(
	const FContextData&          Context,
	const UAdaptiveMemory*       Memory,
	const UDynamicWeightManager* Weights,
	const UAdaptiveConfig*       Config)
{
	FThreatAssessment Result;
	// TODO (Stage 6F): implement full 8-phase formula pipeline
	// T_base → T_cross → T_raw → Confidence → T_smooth → T_final
	LastAssessment = Result;
	return Result;
}

float UThreatCalculator::ComputeTBase(const FContextData& Ctx, const TArray<float>& Weights) const
{
	// TODO (Stage 6F): Σ w_i * f_i(x_i) using DepthrunMath helpers
	return 0.f;
}

float UThreatCalculator::ComputeTCross(const FContextData& Ctx, const TArray<float>& Weights, const UAdaptiveConfig* Cfg) const
{
	// TODO (Stage 6F): w_DH * f_D * f_H + w_WM * W * M_attack
	return 0.f;
}

float UThreatCalculator::ComputeConfidence() const
{
	// TODO (Stage 6F): C = 1 / (1 + σ²) over TRawHistory
	return 1.f;
}

float UThreatCalculator::GetHighThreatThreshold() const
{
	// TODO (Stage 6K): μ + σ from TFinalHistory
	return 0.75f;
}

float UThreatCalculator::GetLowThreatThreshold() const
{
	// TODO (Stage 6K): μ - σ from TFinalHistory
	return 0.25f;
}
