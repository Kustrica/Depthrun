// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UtilityCurves.h"
#include "AdaptiveConfig.h"
#include "Utils/MathUtils.h"

float UUtilityCurves::EvaluateUtility(
	EFSMStateType State, const FThreatAssessment& Threat, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	const float T = Threat.ThreatFinal;

	switch (State)
	{
	case EFSMStateType::Idle:    return EvaluateIdle(T);
	case EFSMStateType::Chase:   return EvaluateChase(T, Context, Cfg);
	case EFSMStateType::Attack:  return EvaluateAttack(T, Cfg);
	case EFSMStateType::Flank:   return EvaluateFlank(T, Context, Cfg);
	case EFSMStateType::Retreat: return EvaluateRetreat(T, Context, Cfg);
	default:                     return 0.f;
	}
}

float UUtilityCurves::EvaluateIdle(float T) const
{
	// max(0, 1 - 6*T²) — Stage 12: steeper drop-off to avoid Idle during combat
	return FMath::Max(0.f, 1.f - 6.f * T * T);
}

float UUtilityCurves::EvaluateChase(float T, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	const float Center = Cfg ? Cfg->ChaseBellCenter : 0.3f;
	const float Width  = Cfg ? Cfg->ChaseBellWidth  : 0.2f;
	float Utility = DepthrunMath::BellCurve(T, Center, Width);

    // Stage 12: Aggressive Push against ranged pressure
    // Personality-aware: Brave/Heroic enemies push harder, Cowards almost never push.
    float PushFactor = 0.4f;
    if (Context.BraveryLevel == EEnemyBravery::Coward) PushFactor = 0.05f;
    else if (Context.BraveryLevel == EEnemyBravery::Heroic) PushFactor = 0.7f;
    
    // Ranged types prefer not to push into melee
    if (Context.CombatStyle == EEnemyCombatStyle::RangedOriented) PushFactor *= 0.4f;

    if (Context.MemoryAggressiveness > 0.4f)
    {
        Utility += Context.MemoryAggressiveness * PushFactor;
    }

    return FMath::Clamp(Utility, 0.f, 1.2f); // Allow slight over-scoring for priority
}

float UUtilityCurves::EvaluateAttack(float T, const UAdaptiveConfig* Cfg) const
{
	const float Center = Cfg ? Cfg->AttackBellCenter : 0.5f;
	const float Width  = Cfg ? Cfg->AttackBellWidth  : 0.18f; // fallback matches AdaptiveConfig.h default
	return DepthrunMath::BellCurve(T, Center, Width);
}

float UUtilityCurves::EvaluateFlank(float T, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	const float Center = Cfg ? Cfg->FlankBellCenter : 0.6f;
	const float Width  = Cfg ? Cfg->FlankBellWidth  : 0.2f;
	// AllyFactor scales Flank utility: solo enemies use FlankSoloBase as floor,
	// full group (AllyCountNorm=1) reaches 1.0.
	// FlankSoloBase=0.75 (was hardcoded 0.5) so solo Flank peak = 0.75 instead of 0.50,
	// allowing it to outbid Attack after 2+ bow shots.
	const float SoloBase   = Cfg ? Cfg->FlankSoloBase : 0.75f;
	const float AllyFactor = SoloBase + ((1.f - SoloBase) * Context.AllyCountNorm);
	
    float Utility = DepthrunMath::BellCurve(T, Center, Width) * AllyFactor;

    // Tactical penalization (only point-blank):
    // Suppress Flank only at hugging distance (< 40 UU) — at that range we're
    // already inside a melee swing, no point circling. Above that, even close
    // distances are valid for ranged Flank (the enemy may be in ranged mode
    // and want to break a clean line of fire).
    if (Context.DistanceToPlayer < 40.f)
    {
        Utility *= 0.4f;
    }

    return Utility;
}

float UUtilityCurves::EvaluateRetreat(float T, const FContextData& Context, const UAdaptiveConfig* Cfg) const
{
	const float Center = Cfg ? Cfg->RetreatSigmoidCenter    : 0.75f;
	const float K      = Cfg ? Cfg->RetreatSigmoidSteepness : 12.f;
	
	float Utility = DepthrunMath::Sigmoid(T, K, Center);

    // Cowards prioritize retreat
    if (Context.BraveryLevel == EEnemyBravery::Coward) Utility *= 1.4f;

    // Cowards retreat to a safer distance
    float SafeZoneDist = 150.f;
    if (Context.BraveryLevel == EEnemyBravery::Coward) SafeZoneDist = 350.f;

	// If distance is safe, penalize retreat utility so enemy stops running
	if (Context.DistanceToPlayer > SafeZoneDist)
	{
		Utility *= 0.4f; 
	}

	return Utility;
}
