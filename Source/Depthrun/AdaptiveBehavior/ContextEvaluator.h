// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "ContextEvaluator.generated.h"

class ABaseEnemy;
class ADepthrunCharacter;
class UAdaptiveConfig;

/**
 * UContextEvaluator — Layer 1
 * Collects all world inputs and normalizes them to [0,1].
 * Output: FContextData — pure data, no decisions made here.
 *
 * Normalization rules:
 *   D_norm = clamp(1 - dist/MaxRange, 0, 1)
 *   H_norm = 1 - (CurrentHP/MaxHP)           → then f_H(x) = x^α
 *   A_norm = clamp(AllyCount/MaxAlly, 0, 1)
 *   R_norm = clamp(EntitiesInRoom/MaxCap, 0, 1)
 *   W_norm = WeaponThreatLookup[WeaponType]
 *
 * Implementation: Stage 6C.
 */
UCLASS()
class DEPTHRUN_API UContextEvaluator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Main entry point. Returns a fully normalized FContextData snapshot.
	 * Called by UAdaptiveBehaviorComponent::EvaluationTick().
	 */
	FContextData EvaluateContext(
		const ABaseEnemy*        Owner,
		const ADepthrunCharacter* Player,
		const UAdaptiveConfig*   Config) const;

private:
	float NormalizeDistance(float RawDistance, float MaxRange) const;
	float NormalizeHealth(float HPRatio, float Exponent) const;
	float EvaluateRoomDensity(const ABaseEnemy* Owner, float MaxCapacity) const;
	int32 CountNearbyAllies(const ABaseEnemy* Owner, float Radius) const;
	float GetWeaponThreatNorm(EWeaponType WeaponType) const;
};
