// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "ContextEvaluator.h"
#include "Enemy/BaseEnemy.h"
#include "Enemy/EnemyHealthComponent.h"
#include "Player/DepthrunCharacter.h"
#include "Player/PlayerCombatComponent.h"
#include "Combat/BaseWeapon.h"
#include "AdaptiveBehavior/AdaptiveConfig.h"
#include "AdaptiveBehavior/AdaptiveMemory.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Utils/MathUtils.h"

FContextData UContextEvaluator::EvaluateContext(
	const ABaseEnemy*        Owner,
	const ADepthrunCharacter* Player,
	const UAdaptiveConfig*   Config) const
{
	FContextData Data;
	if (!Owner || !Player || !Config) return Data;

	// 1. Distance
	float Dist = (Player->GetActorLocation() - Owner->GetActorLocation()).Size();
	Data.DistanceToPlayer = Dist;
	
	// Phase 1: Linear Normalization
	float LinearDistNorm = 1.f - FMath::Clamp(Dist / Config->MaxEngagementRange, 0.f, 1.f);
	
	// Phase 2: Non-linear Transform (Sigmoid)
	Data.DistanceNorm = DepthrunMath::Sigmoid(LinearDistNorm, Config->SigmoidSteepness_Distance, 0.5f);

	// 2. Player Weapon
	if (Player->CombatComponent && Player->CombatComponent->CurrentWeapon) {
		Data.PlayerWeaponType = Player->CombatComponent->CurrentWeapon->GetWeaponType();
	} else {
		Data.PlayerWeaponType = EWeaponType::Melee;
	}
	Data.WeaponThreatNorm = GetWeaponThreatNorm(Data.PlayerWeaponType);

	// 3. Enemy Health
	UEnemyHealthComponent* HC = Owner->FindComponentByClass<UEnemyHealthComponent>();
	if (HC && HC->GetMaxHP() > 0.f) {
		float Ratio = HC->GetCurrentHP() / HC->GetMaxHP();
		Data.EnemyHPRatioNorm = NormalizeHealth(Ratio, Config->HealthPowerExponent);
	}

	// 4. Allies
	Data.NearbyAllyCount = CountNearbyAllies(Owner, Config->AllyCheckRadius);
	Data.AllyCountNorm = FMath::Clamp(static_cast<float>(Data.NearbyAllyCount) / Config->MaxAllyThreshold, 0.f, 1.f);

	// 5. Room Density (approximation using ally count + player for now)
	Data.RoomDensityNorm = EvaluateRoomDensity(Owner, Config->MaxRoomCapacity);

	return Data;
}

FContextData UContextEvaluator::EvaluateContextWithMemory(
	const ABaseEnemy*        Owner,
	const ADepthrunCharacter* Player,
	const UAdaptiveMemory*   Memory,
	const UAdaptiveConfig*   Config) const
{
	FContextData Data = EvaluateContext(Owner, Player, Config);

	// 6. Memory metrics (time-decay aggregates) — needed by Layer 2 T_cross
	if (Memory && Config)
	{
		float Now = Owner ? Owner->GetWorld()->GetTimeSeconds() : 0.f;
		Data.MemoryAggressiveness = Memory->GetDecayedAggressiveness(Now, Config->MemoryDecayLambda);
		Data.MemoryMobility       = Memory->GetDecayedMobility(Now, Config->MemoryDecayLambda);
	}

	return Data;
}

float UContextEvaluator::NormalizeDistance(float RawDistance, float MaxRange) const
{
	// Deprecated in favor of inline sigmoid transform in EvaluateContext
	if (MaxRange <= 0.f) return 0.f;
	return 1.f - FMath::Clamp(RawDistance / MaxRange, 0.f, 1.f);
}

float UContextEvaluator::NormalizeHealth(float HPRatio, float Exponent) const
{
	// Phase 1: Input Normalization (Inverse: 1.0 = almost dead)
	float InverseRatio = FMath::Clamp(1.f - HPRatio, 0.f, 1.f);
	
	// Phase 2: Non-linear Transform (Quadratic/Power)
	return FMath::Pow(InverseRatio, Exponent);
}

float UContextEvaluator::EvaluateRoomDensity(const ABaseEnemy* Owner, float MaxCapacity) const
{
	if (!Owner || MaxCapacity <= 0.f) return 0.f;

	// Approximate density via a larger sphere overlap (e.g. 1500 units) to count entities
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<ABaseEnemy*>(Owner));
	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(
		Owner, Owner->GetActorLocation(), 1500.f, ObjectTypes, APawn::StaticClass(), IgnoreActors, OutActors);

	return FMath::Clamp(OutActors.Num() / MaxCapacity, 0.f, 1.f);
}

int32 UContextEvaluator::CountNearbyAllies(const ABaseEnemy* Owner, float Radius) const
{
	if (!Owner) return 0;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<ABaseEnemy*>(Owner));
	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(
		Owner, Owner->GetActorLocation(), Radius, ObjectTypes, ABaseEnemy::StaticClass(), IgnoreActors, OutActors);

	return OutActors.Num();
}

float UContextEvaluator::GetWeaponThreatNorm(EWeaponType WeaponType) const
{
	switch (WeaponType) {
		case EWeaponType::Melee: return 0.6f;
		case EWeaponType::Ranged: return 0.8f;
		default: return 0.5f;
	}
}
