// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "ContextEvaluator.h"

FContextData UContextEvaluator::EvaluateContext(
	const ABaseEnemy*        Owner,
	const ADepthrunCharacter* Player,
	const UAdaptiveConfig*   Config) const
{
	FContextData Data;
	// TODO (Stage 6C): implement full normalization pipeline
	return Data;
}

float UContextEvaluator::NormalizeDistance(float RawDistance, float MaxRange) const
{
	// TODO (Stage 6C): clamp(1 - dist/MaxRange, 0, 1)
	return 0.f;
}

float UContextEvaluator::NormalizeHealth(float HPRatio, float Exponent) const
{
	// TODO (Stage 6C): f_H(x) = x^Exponent  where x = 1-HPRatio
	return 0.f;
}

float UContextEvaluator::EvaluateRoomDensity(const ABaseEnemy* Owner, float MaxCapacity) const
{
	// TODO (Stage 6C): overlap sphere + clamp
	return 0.f;
}

int32 UContextEvaluator::CountNearbyAllies(const ABaseEnemy* Owner, float Radius) const
{
	// TODO (Stage 6C): SphereOverlapActors of type ABaseEnemy
	return 0;
}

float UContextEvaluator::GetWeaponThreatNorm(EWeaponType WeaponType) const
{
	// TODO (Stage 6C): Melee=0.6, Ranged=0.8 (tunable via Config later)
	return 0.f;
}
