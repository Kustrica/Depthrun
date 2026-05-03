// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChestLootConfig.generated.h"

/**
 * UChestLootConfig
 * Single DataAsset pre-filled with chest reward ranges.
 * Create DA_ChestLootConfig — all fields are already set with sensible defaults.
 * Each chest open grants EXACTLY 3 rewards: Diamonds + Potion + RunItem.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API UChestLootConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UChestLootConfig();

	// ─── Diamonds ────────────────────────────────────────────────────────────

	/** Minimum diamonds per chest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Diamonds",
		meta = (ClampMin = "1"))
	int32 DiamondMin = 5;

	/** Maximum diamonds per chest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Diamonds",
		meta = (ClampMin = "1"))
	int32 DiamondMax = 15;

	// ─── Potions ─────────────────────────────────────────────────────────────

	/** Minimum potions per chest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Potions",
		meta = (ClampMin = "0"))
	int32 PotionMin = 1;

	/** Maximum potions per chest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Potions",
		meta = (ClampMin = "0"))
	int32 PotionMax = 3;

	// ─── Run Item ─────────────────────────────────────────────────────────────

	/**
	 * Probability [0..1] that a run item is awarded.
	 * Set to 1.0 to always give an item, 0.0 to never.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot|Item",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ItemDropChance = 0.7f;

	/** Roll a random diamond amount in [DiamondMin, DiamondMax]. */
	int32 RollDiamonds() const;

	/** Roll a random potion amount in [PotionMin, PotionMax]. */
	int32 RollPotions() const;
};
