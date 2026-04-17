// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoomGeneratorSubsystem.generated.h"

class ARoomBase;
class URoomTemplate;

/**
 * URoomGeneratorSubsystem
 * UWorldSubsystem that generates the roguelike room sequence on level load.
 *
 * Algorithm (Stage 8B): simple room graph, BSP or Random Walk.
 *   1. Generate room sequence (N rooms + 1 boss room)
 *   2. Assign types (Start, Combat, Rest, Boss)
 *   3. Spawn ARoomBase actors at grid positions
 *   4. Place ARoomTransitionVolume at exits
 *
 * Accessed via GetWorld()->GetSubsystem<URoomGeneratorSubsystem>()
 */
UCLASS()
class DEPTHRUN_API URoomGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Kick off room generation for the current level. */
	UFUNCTION(BlueprintCallable, Category = "RoomGen")
	void GenerateRooms(int32 RoomCount = 7);

	/** Called by ARoomTransitionVolume when player enters a transition area. */
	UFUNCTION(BlueprintCallable, Category = "RoomGen")
	void OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex);

	/** All generated rooms in sequence. */
	const TArray<ARoomBase*>& GetRooms() const { return GeneratedRooms; }

	/** Index of the currently active room. */
	int32 GetCurrentRoomIndex() const { return CurrentRoomIndex; }

	// ─── Room pool (assign templates in GameMode or LevelBlueprint) ──────────

	UPROPERTY(EditDefaultsOnly, Category = "RoomGen")
	TArray<TObjectPtr<URoomTemplate>> CombatRoomTemplates;

	UPROPERTY(EditDefaultsOnly, Category = "RoomGen")
	TObjectPtr<URoomTemplate> BossRoomTemplate;

private:
	void SpawnRoom(URoomTemplate* Template, const FVector& Location);
	void ActivateRoom(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<ARoomBase>> GeneratedRooms;

	int32 CurrentRoomIndex = 0;
};
