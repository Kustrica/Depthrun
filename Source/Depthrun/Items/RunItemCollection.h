// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RunItemCollection.generated.h"

class URunItemConfig;

/**
 * URunItemCollection
 * Single DataAsset containing all available run items.
 * Create one DA_RunItemCollection in Content/Data/Items/ and fill the array.
 * Referenced from BP_DepthrunCharacter for console-command testing and Hub UI.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API URunItemCollection : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All available run items. Each entry is a URunItemConfig DataAsset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	TArray<TObjectPtr<URunItemConfig>> Items;

	/** Find item by display name (case-insensitive). Returns nullptr if not found. */
	UFUNCTION(BlueprintPure, Category = "Items")
	URunItemConfig* FindByName(const FString& Name) const;
};
