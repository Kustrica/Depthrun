// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoomGeneratorSubsystem.generated.h"

class ARoomBase;
class URoomTemplate;

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

	/** Called by ARoomBase when player enters its bounds. */
	void OnPlayerEnteredRoom(ARoomBase* Room);

	/** Initialization from GameMode. */
	void SetTemplates(
		URoomTemplate* Start, 
		URoomTemplate* Boss, 
		const TArray<URoomTemplate*>& Combat,
		const TArray<URoomTemplate*>& Treasure,
		const TArray<URoomTemplate*>& Rest
	);

	/** Called by transition volumes. */
	void OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex);

	const TArray<ARoomBase*>& GetRooms() const { return GeneratedRooms; }
	int32 GetCurrentRoomIndex() const { return CurrentRoomIndex; }
	int32 GetTotalRooms() const { return GeneratedRooms.Num(); }

	/** Returns the currently active room actor, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "RoomGen")
	ARoomBase* GetCurrentActiveRoom() const;

protected:
	UPROPERTY()
	TObjectPtr<URoomTemplate> StartTemplate;
	
	UPROPERTY()
	TObjectPtr<URoomTemplate> BossTemplate;

	UPROPERTY()
	TArray<TObjectPtr<URoomTemplate>> CombatPool;

	UPROPERTY()
	TArray<TObjectPtr<URoomTemplate>> TreasurePool;

	UPROPERTY()
	TArray<TObjectPtr<URoomTemplate>> RestPool;

private:
	void ActivateRoom(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<ARoomBase>> GeneratedRooms;

	int32 CurrentRoomIndex = 0;
};
