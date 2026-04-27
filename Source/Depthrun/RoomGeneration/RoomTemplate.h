// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DungeonTypes.h"
#include "Engine/DataAsset.h"
#include "RoomTemplate.generated.h"

class ATrapdoorActor;
class AChestActor;

UCLASS(BlueprintType)
class DEPTHRUN_API URoomTemplate : public UDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, Category = "Layout")
  TObjectPtr<class UPaperTileMap> TileMapAsset;

  UPROPERTY(EditAnywhere, Category = "Layout")
  ERoomType RoomType = ERoomType::Combat;

  /** Uniform world scale used by procedural spawn math for this template. */
  UPROPERTY(EditAnywhere, Category = "Layout")
  float WorldScale = 2.6f;

  // ─── Тайлы ──────────────────────────────────────────────────────────────
  // Each doorway has 2 tile slots (A = first cell, B = second cell).
  // Assign the correct PackedTileIndex from your tileset for each slot.

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowTop_A;    // tile position (3, 0)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowTop_B;    // tile position (4, 0)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowBottom_A; // tile position (3, 5)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowBottom_B; // tile position (4, 5)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowLeft_A;   // tile position (0, 2)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowLeft_B;   // tile position (0, 3)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowRight_A;  // tile position (7, 2)

  UPROPERTY(EditAnywhere, Category = "Tiles|DoorShadows")
  FRoomTileInfo DoorShadowRight_B;  // tile position (7, 3)

  // ─── Основные классы ───────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class ADoorActor> DoorClass;

  /** Exit trapdoor class - spawned when boss room is cleared. Must derive from ATrapdoorActor for overlap events. */
  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<ATrapdoorActor> TrapdoorClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<class AActor> TorchClass;

  UPROPERTY(EditAnywhere, Category = "Props|Classes")
  TSubclassOf<AChestActor> ChestClass;

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

  /** [Deprecated] Doors now use PropRotation. Kept only for asset backwards compatibility. */
  UPROPERTY()
  FRotator DoorRotation = FRotator(-90.f, 0.f, 90.f);

  /** Master offset for ALL procedurally spawned actors relative to the Room's zero point. Use this to center props/enemies within your TileMap. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  FVector SpawnOffset = FVector::ZeroVector;

  /** Fine-tune TileMap visual position relative to the geometric room center (World X / World Y).
   *  Does NOT affect wall colliders or prop/enemy spawn positions — only the tilemap art. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  FVector2D TileMapVisualOffset = FVector2D(-20.f, 15.f);

  /** Z-Height for the TileMap. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float TileMapZ = 0.0f;

  /** Z-Height for all props (Decor, Torches, Chests). */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float PropsZ = 1.0f;

  /** Z-Height for the exit trapdoor. Higher than PropsZ to avoid sinking below tilemap. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float TrapdoorZ = 4.0f;

  /** Z-Height for doors (spawn + collision plane).
   *  Default 3.0 places the door above floor tiles and props to avoid Z-fighting. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float DoorLockedZ = 3.0f;

  /** Z-Height for enemies (spawn + plane constraint). */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float EnemyLockedZ = 1.0f;

  /** Z-Height to spawn AND lock the player at during gameplay (prevents Z drift).
   *  This is the single source of truth for player height — no clamping applied. */
  UPROPERTY(EditAnywhere, Category = "Offsets")
  float PlayerLockedZ = 1.0f;

  /** If true, room generation creates extra procedural wall BoxComponents. */
  UPROPERTY(EditAnywhere, Category = "Collision")
  bool bUseProceduralWallColliders = false;

  /** If true, floor obstacles use each sprite's own collision geometry. */
  UPROPERTY(EditAnywhere, Category = "Collision")
  bool bUseObstacleSpriteCollision = true;

  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  int32 MinProps = 2;

  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  int32 MaxProps = 6;

  /** Tile coordinates for torches. */
  UPROPERTY(EditAnywhere, Category = "Props|Spawning")
  TArray<FIntPoint> TorchSpots = {{1, 0}, {2, 0}, {5, 0}, {6, 0}};
};
