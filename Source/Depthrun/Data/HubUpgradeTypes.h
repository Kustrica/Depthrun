// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "HubUpgradeTypes.generated.h"

/** Meta-progression upgrade types available in the Hub. */
UENUM(BlueprintType)
enum class EHubUpgrade : uint8
{
	Damage      UMETA(DisplayName = "Damage"),
	Range       UMETA(DisplayName = "Range"),
	ArrowCount  UMETA(DisplayName = "Arrow Count"),
	MaxHP       UMETA(DisplayName = "Max HP")
};

namespace HubUpgradeConfig
{
	/** Maximum level for any upgrade. */
	static constexpr int32 MAX_LEVEL = 5;

	/**
	 * Calculate upgrade cost for the next level.
	 * Formula: floor(50 * 1.6^CurrentLevel)
	 * Cost table: 50, 80, 128, 204, 327 (for levels 0→1 through 4→5)
	 */
	inline int32 GetUpgradeCost(int32 CurrentLevel)
	{
		if (CurrentLevel >= MAX_LEVEL)
		{
			return -1; // Max level reached
		}
		return FMath::FloorToInt(50.f * FMath::Pow(1.6f, static_cast<float>(CurrentLevel)));
	}

	/** Get the stat multiplier for a given upgrade level. */
	inline float GetDamageMultiplier(int32 Level)
	{
		return 1.0f + (Level * 0.05f); // +5% per level
	}

	inline float GetMeleeRangeMultiplier(int32 Level)
	{
		return 1.0f + (Level * 0.10f); // +10% per level
	}

	inline int32 GetBaseProjectileCount(int32 Level)
	{
		return 3 + Level; // 3 → 4 → 5 (max 5 at level 2 actually... but we allow up to 5)
	}

	inline float GetMaxHPBonus(int32 Level)
	{
		return static_cast<float>(Level * 10); // +10 HP per level
	}
}
