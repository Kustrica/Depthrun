// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "RunItemTypes.generated.h"

/**
 * ERunItemEffect
 * All possible per-run item effects. New effects = new enum value only.
 * Implementation dispatched via switch in URunItemInventory::ApplyItems().
 *
 * Sword effects:
 *   MeleeExtendedRange  — increases melee weapon hit radius
 *   MeleeDoubleSwing    — triggers a second swing immediately after the first
 *
 * Bow effects:
 *   ArrowRicochet       — arrow bounces once off a wall or enemy
 *   ArrowPierce         — arrow passes through enemies without destroying itself
 */
UENUM(BlueprintType)
enum class ERunItemEffect : uint8
{
	None               UMETA(DisplayName = "None"),

	// Universal
	FlatDamage         UMETA(DisplayName = "Power: Flat Damage"),

	// Melee (Sword)
	MeleeExtendedRange UMETA(DisplayName = "Melee: Extended Range"),
	MeleeDoubleSwing   UMETA(DisplayName = "Melee: Double Swing"),

	// Ranged (Bow)
	ArrowRicochet      UMETA(DisplayName = "Arrow: Ricochet"),
	ArrowPierce        UMETA(DisplayName = "Arrow: Pierce"),

	// Character stats (Any weapon)
	BonusMaxHP           UMETA(DisplayName = "Stat: Bonus Max HP"),
	BonusMoveSpeed       UMETA(DisplayName = "Stat: Bonus Move Speed"),
	BonusProjectileCount UMETA(DisplayName = "Stat: Bonus Projectile Count")
};

/** Which weapon type an item modifies. None = universal. */
UENUM(BlueprintType)
enum class EItemTargetWeapon : uint8
{
	Any    UMETA(DisplayName = "Any"),
	Melee  UMETA(DisplayName = "Melee (Sword)"),
	Ranged UMETA(DisplayName = "Ranged (Bow)")
};
