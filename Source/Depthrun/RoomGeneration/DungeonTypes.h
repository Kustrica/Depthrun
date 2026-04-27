// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "DungeonTypes.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
    Start    UMETA(DisplayName = "Start"),
    Combat   UMETA(DisplayName = "Combat"),
    Boss     UMETA(DisplayName = "Boss"),
    Treasure UMETA(DisplayName = "Treasure")
};

UENUM(BlueprintType)
enum class ERoomShape : uint8
{
    Single      UMETA(DisplayName = "1x1 Single"),
    Horizontal  UMETA(DisplayName = "2x1 Horizontal"),
    Vertical    UMETA(DisplayName = "1x2 Vertical"),
    Big         UMETA(DisplayName = "2x2 Big"),
    LShape      UMETA(DisplayName = "L-Shape")
};

/**
 * EEnemySpawnState
 * Tracks per-spawn-point occupancy during room activation.
 */
UENUM(BlueprintType)
enum class EEnemySpawnState : uint8
{
    Free     UMETA(DisplayName = "Free"),
    Occupied UMETA(DisplayName = "Occupied")
};

USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ABaseEnemy> EnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FRoomTileInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UPaperTileSet> TileSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PackedTileIndex = -1;

    /** Rotation in degrees (0, 90, 180, 270) for tile orientation.
     *  Used for door shadow tiles to face shadows toward walls. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Rotation = 0;
};
