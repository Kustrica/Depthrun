// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/RunItemTypes.h"
#include "RunItemConfig.generated.h"

/**
 * URunItemConfig
 * DataAsset describing one run item. All items are defined as DA assets in Editor.
 * Items have no sprites — they are ability modifiers only.
 *
 * One item = one ERunItemEffect applied to a specific weapon type.
 * Created in Editor: Content/Data/Items/DA_Item_XXX
 *
 * Stage 3G: created as skeleton; effects applied in URunItemInventory.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API URunItemConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemName = TEXT("Unknown Item");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	ERunItemEffect Effect = ERunItemEffect::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemTargetWeapon TargetWeapon = EItemTargetWeapon::Any;

	// ─── Numeric effect parameters (used by specific effects) ────────────────

	/** MeleeExtendedRange: multiplier on hit sphere radius (default 1.0 = no change). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Parameters",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float RangeMultiplier = 1.5f;

	/** ArrowRicochet: max bounce count. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Parameters",
		meta = (ClampMin = "1", ClampMax = "3"))
	int32 RicochetCount = 1;
};
