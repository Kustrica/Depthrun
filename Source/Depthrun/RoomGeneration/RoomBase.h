// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DungeonTypes.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"

class UBoxComponent;
class UPaperTileMapComponent;
class URoomTemplate;

/**
 * ARoomBase
 * Container actor for a generated room.
 */
UCLASS()
class DEPTHRUN_API ARoomBase : public AActor {
  GENERATED_BODY()

public:
  ARoomBase();

protected:
  virtual void BeginPlay() override;

public:
  /** Initialize room from template and setup doors. */
  void SetupRoom(URoomTemplate *Template, bool bHasTop, bool bHasBottom,
                 bool bHasLeft, bool bHasRight);

  UFUNCTION(BlueprintCallable, Category = "Room")
  void ActivateRoom();

  UFUNCTION(BlueprintCallable, Category = "Room")
  void DeactivateRoom();

  UFUNCTION(BlueprintPure, Category = "Room")
  bool IsCleared() const { return bIsCleared; }

  // ─── Components ──────────────────────────────────────────────────────────

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
  TObjectPtr<UPaperTileMapComponent> TileMapComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
  TObjectPtr<UBoxComponent> RoomBounds;

private:
  /** Called when player overlaps RoomBounds. */
  UFUNCTION()
  void OnRoomEntry(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                   UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                   bool bFromSweep, const FHitResult &SweepResult);

  /** Periodic check to see if all spawned enemies are dead. */
  void CheckEnemiesStatus();

  void SpawnEnemies();
  void GenerateProps(bool bHasTop, bool bHasBottom, bool bHasLeft,
                     bool bHasRight);
  void TrySpawnChest();

  UPROPERTY()
  TObjectPtr<URoomTemplate> MyTemplate;

  /** Use TWeakObjectPtr to avoid keeping dead enemies in memory. */
  TArray<TWeakObjectPtr<AActor>> SpawnedEnemies;

  UPROPERTY()
  TArray<AActor *> SpawnedDoors;

  FTimerHandle TimerHandle_CheckEnemies;

  bool bIsCleared = false;
  bool bHasGeneratedChest = false;

  // Helper for tile placement
  void SetTileInLayer(int32 Layer, int32 X, int32 Y,
                      const FRoomTileInfo &TileInfo);

protected:
  /** Tiles that already have props (like torches) to avoid overlapping. */
  TSet<FIntPoint> OccupiedTiles;
};
