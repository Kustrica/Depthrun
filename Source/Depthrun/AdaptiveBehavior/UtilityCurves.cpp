// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UtilityCurves.h"
#include "AdaptiveConfig.h"
#include "Utils/MathUtils.h"

float UUtilityCurves::EvaluateUtility(
	EFSMStateType State, float T, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	switch (State)
	{
	case EFSMStateType::Idle:    return EvaluateIdle(T);
	case EFSMStateType::Chase:   return EvaluateChase(T, Cfg);
	case EFSMStateType::Attack:  return EvaluateAttack(T, Cfg);
	case EFSMStateType::Flank:   return EvaluateFlank(T, Context.AllyCountNorm, Cfg);
	case EFSMStateType::Retreat: return EvaluateRetreat(T, Cfg);
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
	// BellCurve(T, 0.6, 0.2) * A_norm
	const float Center = Cfg ? Cfg->FlankBellCenter : 0.6f;
	const float Width  = Cfg ? Cfg->FlankBellWidth  : 0.2f;
	return DepthrunMath::BellCurve(T, Center, Width) * ANorm;
}

float UUtilityCurves::EvaluateRetreat(float T, const UAdaptiveConfig* Cfg) const
{
	// Sigmoid(T, center=0.75, k=12)
	const float Center = Cfg ? Cfg->RetreatSigmoidCenter    : 0.75f;
	const float K      = Cfg ? Cfg->RetreatSigmoidSteepness : 12.f;
	return DepthrunMath::Sigmoid(T, K, Center);
}
