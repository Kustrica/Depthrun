// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DungeonTypes.h"
#include "Engine/DataAsset.h"
#include "RoomTemplate.generated.h"

UCLASS(BlueprintType)
class DEPTHRUN_API URoomTemplate : public UDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, Category = "Layout")
  TObjectPtr<class UPaperTileMap> TileMapAsset;

  UPROPERTY(EditAnywhere, Category = "Layout")
  ERoomType RoomType = ERoomType::Combat;

  // ─── Тайлы ──────────────────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorFloorShadowTop;

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorFloorShadowBottom;

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorFloorShadowLeft;

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorFloorShadowRight;

  // ─── Основные классы ───────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class ADoorActor> DoorClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> TrapdoorClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> TorchClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> ChestClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> SkullDecorClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> BonesDecorClass;

  /** Класс для торшера/препятствия. */
  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> FloorObstacleClass;

  // ─── Спрайты ────────────────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, Category = "Props|Sprites")
  TObjectPtr<class UPaperSprite> HorizontalDoorSprite;

  UPROPERTY(EditAnywhere, Category = "Props|Sprites")
  TObjectPtr<class UPaperSprite> VerticalDoorSprite;

  /** 4 варианта спрайтов для торшера. */
  UPROPERTY(EditAnywhere, Category = "Props|Sprites")
  TArray<TObjectPtr<class UPaperSprite>> ObstacleVariants;

  // ─── Настройки ──────────────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, Category = "Combat")
  TArray<FEnemySpawnInfo> PotentialEnemies;

  UPROPERTY(EditAnywhere, Category = "Combat")
  int32 MinEnemies = 2;

  UPROPERTY(EditAnywhere, Category = "Combat")
  int32 MaxEnemies = 4;

  UPROPERTY(EditAnywhere, Category = "Chances")
  float ChestSpawnChance = 10.0f;

  UPROPERTY(EditAnywhere, Category = "Chances")
  float TorchSpawnChance = 50.0f;

  // ─── Global Rotation Settings ──────────────────────────────────────────
  
  /** Rotation for the floor/walls (TileMap). */
  UPROPERTY(EditAnywhere, Category = "Rotation")
  FRotator TileMapRotation = FRotator(-90.f, 0.f, 90.f);

  /** Rotation for props (torches, decor, doors). */
  UPROPERTY(EditAnywhere, Category = "Rotation")
  FRotator PropRotation = FRotator(-90.f, 0.f, 90.f);

  /** Rotation for enemies. */
  UPROPERTY(EditAnywhere, Category = "Rotation")
  FRotator EnemyRotation = FRotator(-90.f, 0.f, 90.f);

  /** Rotation for doors specifically, as they might need to face outward. */
  UPROPERTY(EditAnywhere, Category = "Rotation")
  FRotator DoorRotation = FRotator(-90.f, 0.f, 90.f);

  /** Master offset for ALL procedurally spawned actors relative to the Room's zero point. Use this to center props/enemies within your TileMap. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  FVector SpawnOffset = FVector(0.f, 0.f, 0.f);

  /** Z-Height for the TileMap. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float TileMapZ = 0.0f;

  /** Z-Height for all props (Decor, Torches, Chests). */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float PropsZ = 1.0f;

  /** Z-Height for doors. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float DoorZ = 1.0f;

  /** Z-Height for enemies. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float EnemyZ = 1.0f;

  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  int32 MinProps = 2;

  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  int32 MaxProps = 6;

  /** Tile coordinates for torches. */
  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  TArray<FIntPoint> TorchSpots = {{1, 0}, {2, 0}, {5, 0}, {6, 0}};
};
