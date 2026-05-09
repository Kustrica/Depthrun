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
	/** Default maximum level for most upgrades. */
	static constexpr int32 MAX_LEVEL = 5;

	/** Maximum level for the ArrowCount upgrade (multishot — powerful, scarce). */
	static constexpr int32 ARROW_COUNT_MAX_LEVEL = 3;

	/** Per-upgrade maximum level. Returns ARROW_COUNT_MAX_LEVEL for ArrowCount. */
	inline int32 GetMaxLevel(EHubUpgrade Upgrade)
	{
		if (Upgrade == EHubUpgrade::ArrowCount) return ARROW_COUNT_MAX_LEVEL;
		return MAX_LEVEL;
	}

	/**
	 * Base cost formula used by most upgrades.
	 * Formula: floor(50 * 1.6^CurrentLevel)
	 * Cost table: 50, 80, 128, 204, 327 (levels 0→1 through 4→5)
	 */
	inline int32 GetUpgradeCost(int32 CurrentLevel)
	{
		return FMath::FloorToInt(50.f * FMath::Pow(1.6f, static_cast<float>(CurrentLevel)));
	}

	/**
	 * Type-aware cost lookup. ArrowCount uses a steeper curve because it is
	 * strictly limited to 3 levels but grants a powerful multishot bonus.
	 * ArrowCount cost table: 150, 300, 600 (levels 0→1, 1→2, 2→3)
	 */
	inline int32 GetUpgradeCostForType(EHubUpgrade Upgrade, int32 CurrentLevel)
	{
		CurrentLevel = FMath::Max(CurrentLevel, 0); // Guard against corrupted data
		if (CurrentLevel >= GetMaxLevel(Upgrade)) return -1; // max reached
		if (Upgrade == EHubUpgrade::ArrowCount)
			return FMath::FloorToInt(150.f * FMath::Pow(2.0f, static_cast<float>(CurrentLevel)));
		return GetUpgradeCost(CurrentLevel);
	}

	/** Get the stat multiplier for a given upgrade level. */
	inline float GetDamageMultiplier(int32 Level)
	{
		Level = FMath::Max(Level, 0); // Guard against corrupted data
		return 1.0f + (Level * 0.05f); // +5% per level
	}

	inline float GetMeleeRangeMultiplier(int32 Level)
	{
		Level = FMath::Max(Level, 0); // Guard against corrupted data
		return 1.0f + (Level * 0.10f); // +10% per level
	}

	/**
	 * Base shots fired per arrow release for a given ArrowCount hub level.
	 * Level 0 = 1 arrow (default), level 1 = 2, level 2 = 3, level 3 = 4.
	 * The Multishot run item can add +1 on top, for a maximum of 5.
	 */
	inline int32 GetBaseProjectileCount(int32 Level)
	{
		Level = FMath::Max(Level, 0); // Guard against corrupted data
		return 1 + Level; // 1 → 2 → 3 → 4 (max from hub at level 3)
	}

	inline float GetMaxHPBonus(int32 Level)
	{
		Level = FMath::Max(Level, 0); // Guard against corrupted data
		return static_cast<float>(Level * 10); // +10 HP per level
	}
}
