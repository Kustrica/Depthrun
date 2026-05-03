// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/RunItemTypes.h"
#include "RunItemCollection.generated.h"

/**
 * FRunItemData
 * Inline item definition — no separate DataAsset needed.
 * All fields are editable in the single DA_RunItemCollection asset.
 */
USTRUCT(BlueprintType)
struct DEPTHRUN_API FRunItemData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FString ItemName = TEXT("Unknown");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	ERunItemEffect Effect = ERunItemEffect::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemTargetWeapon TargetWeapon = EItemTargetWeapon::Any;

	/** ArrowRicochet: bounce count */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Parameters",
		meta = (ClampMin = "1", ClampMax = "5"))
	int32 RicochetCount = 1;

	/** MeleeExtendedRange: hit radius multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Parameters",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float RangeMultiplier = 1.0f;

	/** BonusMaxHP / BonusMoveSpeed / BonusProjectileCount: numeric boost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Parameters")
	float NumericValue = 0.f;
};

/**
 * URunItemCollection
 * Single DataAsset with all run items pre-filled in C++.
 * Create DA_RunItemCollection — the Items array is already populated with
 * all 7 default entries. Tweak numbers directly in the asset editor.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API URunItemCollection : public UDataAsset
{
	GENERATED_BODY()

public:
	URunItemCollection();

	/** All run items. Pre-filled with 7 defaults; edit values freely. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	TArray<FRunItemData> Items;

	/** Find item by partial name match (case-insensitive). Returns index or -1. */
	int32 FindIndexByName(const FString& Name) const;

	/** Returns pointer to item data by partial name, or nullptr. */
	const FRunItemData* FindByName(const FString& Name) const;
};
