// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy/EnemyTypes.h"
#include "RoomBase.generated.h"

class UBoxComponent;

/**
 * ARoomBase
 * Base room actor. Owns bounds, spawn points, exit points.
 * Enemy spawning and door logic are triggered by URoomGeneratorSubsystem.
 * Implementation: Stage 8A.
 */
UCLASS()
class DEPTHRUN_API ARoomBase : public AActor
{
	GENERATED_BODY()

public:
	ARoomBase();

protected:
	virtual void BeginPlay() override;

public:
	/** Activate room: spawn enemies, close doors. */
	UFUNCTION(BlueprintCallable, Category = "Room")
	void ActivateRoom();

	/** Deactivate room: open doors (called when all enemies in room are dead). */
	UFUNCTION(BlueprintCallable, Category = "Room")
	void DeactivateRoom();

	/** Returns true if all enemies in the room have been cleared. */
	UFUNCTION(BlueprintPure, Category = "Room")
	bool IsCleared() const;

	// ─── Config ──────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	ERoomType RoomType = ERoomType::Combat;

	/** Spawn point transforms for enemy placement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<FVector> SpawnPoints;

	/** Door/exit positions for transition triggers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<FVector> ExitPoints;

	// ─── Components ──────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room")
	TObjectPtr<UBoxComponent> RoomBounds;

private:
	TArray<TWeakObjectPtr<AActor>> SpawnedEnemies;
};
