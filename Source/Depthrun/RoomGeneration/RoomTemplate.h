// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enemy/EnemyTypes.h"
#include "RoomTemplate.generated.h"

class ARoomBase;

/**
 * URoomTemplate
 * DataAsset describing a room layout for use by URoomGeneratorSubsystem.
 * Assign in Editor to populate room pools.
 * Implementation: Stage 8A.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API URoomTemplate : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Friendly name shown in Editor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	FName TemplateName = TEXT("Room_Default");

	/** Type of room this template represents. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	ERoomType RoomType = ERoomType::Combat;

	/** Reference to an ARoomBase Blueprint asset to spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	TSubclassOf<ARoomBase> RoomClass;

	/** Size hint (used by generator for placement grid). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room")
	FVector2D RoomSize = FVector2D(2000.f, 2000.f);

	/** Difficulty rating [0,1]. Higher = more/harder enemies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DifficultyRating = 0.5f;

	/** Enemy classes to spawn in this room template. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Enemies")
	TArray<TSubclassOf<AActor>> EnemyClasses;

	/** Max enemies this template spawns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Room|Enemies")
	int32 MaxEnemyCount = 3;
};
