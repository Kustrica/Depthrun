// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class ABaseWeapon;

/** Fired by this component when the player successfully deals damage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealt, AActor*, Target, float, Damage);

/**
 * UPlayerCombatComponent
 * Manages the player's equipped weapon. Broadcasts OnDamageDealt so that
 * enemy DynamicWeightManager can apply +1 reward.
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

	/** Fire / swing the currently equipped weapon. */
	void Attack();

	/** Replace the current weapon (destroys old one). */
	void EquipWeapon(ABaseWeapon* NewWeapon);

	/** Broadcast when damage is dealt — subscribed by enemy AdaptiveBehaviorComponent. */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageDealt OnDamageDealt;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ABaseWeapon> CurrentWeapon;
};
