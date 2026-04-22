// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UtilityCurves.h"
#include "AdaptiveConfig.h"
#include "Utils/MathUtils.h"

float UUtilityCurves::EvaluateUtility(
	EFSMStateType State, const FThreatAssessment& Threat, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	const float T = Threat.ThreatFinal;
	const float Dist = Context.DistanceToPlayer;
	
	// Stage 6K: Adaptive Center Shifting
	// Shift centers based on how much the current mean threat (μ) deviates from the default threat.
	// If the world is generally "scarier" now, we shift centers right, becoming more tolerant.
	float AdaptiveShift = 0.f;
	if (Cfg && Threat.AdaptiveMeanThreat > 0.f)
	{
		AdaptiveShift = Threat.AdaptiveMeanThreat - Cfg->DefaultThreat;
	}

	switch (State)
	{
	case EFSMStateType::Idle:    return EvaluateIdle(T);
	case EFSMStateType::Chase:   return EvaluateChase(T + AdaptiveShift, Cfg);
	case EFSMStateType::Attack:  return EvaluateAttack(T + AdaptiveShift, Cfg);
	case EFSMStateType::Flank:   return EvaluateFlank(T + AdaptiveShift, Context.AllyCountNorm, Cfg);
	case EFSMStateType::Retreat: return EvaluateRetreat(T, Context, Cfg); // Retreat is usually absolute
	default:                     return 0.f;
	}
}

float UUtilityCurves::EvaluateIdle(float T) const
{
	// max(0, 1 - 4*T²)
	return FMath::Max(0.f, 1.f - 4.f * T * T);
}

float UUtilityCurves::EvaluateChase(float T, const UAdaptiveConfig* Cfg) const
{
	// BellCurve(T, center, width) — TODO (Stage 6H): use Cfg parameters
	const float Center = Cfg ? Cfg->ChaseBellCenter : 0.3f;
	const float Width  = Cfg ? Cfg->ChaseBellWidth  : 0.2f;
	return DepthrunMath::BellCurve(T, Center, Width);
}

float UUtilityCurves::EvaluateAttack(float T, const UAdaptiveConfig* Cfg) const
{
	const float Center = Cfg ? Cfg->AttackBellCenter : 0.5f;
	const float Width  = Cfg ? Cfg->AttackBellWidth  : 0.25f;
	return DepthrunMath::BellCurve(T, Center, Width);
}

float UUtilityCurves::EvaluateFlank(float T, float ANorm, const UAdaptiveConfig* Cfg) const
{
	// Commercial Fix: Don't multiply by ANorm directly (kills solo flanking).
	// Use (0.5 + 0.5 * ANorm) so solo enemies have at least 50% utility.
	const float Center = Cfg ? Cfg->FlankBellCenter : 0.6f;
	const float Width  = Cfg ? Cfg->FlankBellWidth  : 0.2f;
	const float AllyFactor = 0.5f + (0.5f * ANorm);
	return DepthrunMath::BellCurve(T, Center, Width) * AllyFactor;
}

float UUtilityCurves::EvaluateRetreat(float T, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	// Sigmoid(T, center=0.75, k=12)
	const float Center = Cfg ? Cfg->RetreatSigmoidCenter    : 0.75f;
	const float K      = Cfg ? Cfg->RetreatSigmoidSteepness : 12.f;
	
	// Commercial Fix: If T is high but we are far, reduce retreat utility 
	// to allow transition back to Attack (Ranged).
	float Utility = DepthrunMath::Sigmoid(T, K, Center);

	// If distance is safe (> 60), penalize retreat utility so enemy stops running
	if (Context.DistanceToPlayer > 60.f)
	{
		Utility *= 0.5f; // Reduce desire to run further by half
	}

	return Utility;
}
