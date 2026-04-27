// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerEconomy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiamondsChanged, int32, OldValue, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPotionsChanged, int32, OldValue, int32, NewValue);

/**
 * UPlayerEconomy
 * Tracks run-scoped currency (diamonds) and consumables (health potions).
 * Broadcasts delegates for UI updates. Handles potion use with cooldown.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEPTHRUN_API UPlayerEconomy : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerEconomy();

	/** Current diamonds collected this run. */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 RunDiamonds = 0;

	/** Current health potions held. */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 HealthPotions = 0;

	/** Broadcast when diamonds change. */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Events")
	FOnDiamondsChanged OnDiamondsChanged;

	/** Broadcast when potions change. */
	UPROPERTY(BlueprintAssignable, Category = "Economy|Events")
	FOnPotionsChanged OnPotionsChanged;

	/** Add diamonds and broadcast. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddDiamonds(int32 Amount);

	/** Add potions and broadcast. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddPotions(int32 Amount);

	/**
	 * Use a health potion if available and not on cooldown.
	 * @return true if potion was used, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool UsePotion();

	/**
	 * Called on player death. Converts 50% of RunDiamonds to profile,
	 * clears run inventory. Delegates actual save to external caller.
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void OnPlayerDeath();

	/**
	 * Called on successful run exit to Hub. Converts 100% of RunDiamonds
	 * to profile, clears run inventory.
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void OnRunExitToHub();

	/** Clear all run economy (on death/exit). */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ClearRunEconomy();

	/** Check if potion use is on cooldown. */
	UFUNCTION(BlueprintPure, Category = "Economy")
	bool IsPotionOnCooldown() const;

private:
	/** Time of last potion use (in seconds). */
	float LastPotionUseTime = -999.f;

	/** Cooldown between potion uses (seconds). */
	static constexpr float PotionCooldown = 0.5f;

	/** HP restored per potion. */
	static constexpr float PotionHealAmount = 50.f;
};
