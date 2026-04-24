// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonTypes.h"
#include "RoomTemplate.generated.h"

/**
 * URoomTemplate
 * Configuration for a procedural room.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API URoomTemplate : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Layout")
    TObjectPtr<class UPaperTileMap> TileMapAsset;

    UPROPERTY(EditAnywhere, Category = "Layout")
    ERoomType RoomType = ERoomType::Combat;

    UPROPERTY(EditAnywhere, Category = "Layout")
    ERoomShape Shape = ERoomShape::Single;

    // ─── Tile Replacement ──────────────────────────────────────────────────

    /** Tile used to fill a door slot if there is no connection. */
    UPROPERTY(EditAnywhere, Category = "Tiles|Replacement")
    FRoomTileInfo WallTile;

    /** Tile used for the floor in a door slot if the door is open. */
    UPROPERTY(EditAnywhere, Category = "Tiles|Replacement")
    FRoomTileInfo DoorFloorTile;

    // ─── Prop Classes ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<class AActor> DoorClass;

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<class AActor> TorchClass;

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<class AActor> ChestClass;

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<class AActor> BoneDecorClass;

    // ─── Spawning Logic ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, Category = "Generation|Enemies")
    TArray<FEnemySpawnInfo> PotentialEnemies;

    UPROPERTY(EditAnywhere, Category = "Generation|Enemies")
    int32 MinEnemies = 2;

    UPROPERTY(EditAnywhere, Category = "Generation|Enemies")
    int32 MaxEnemies = 4;

    /** Chance [0-100] to spawn a chest after room clear. */
    UPROPERTY(EditAnywhere, Category = "Generation|Chances", meta=(ClampMin="0", ClampMax="100"))
    float ChestSpawnChance = 10.0f;

    /** Chance [0-100] per valid spot to spawn a torch. */
    UPROPERTY(EditAnywhere, Category = "Generation|Chances", meta=(ClampMin="0", ClampMax="100"))
    float TorchSpawnChance = 50.0f;
};
