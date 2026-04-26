// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DepthrunGameMode.generated.h"

UCLASS()
class DEPTHRUN_API ADepthrunGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADepthrunGameMode();

protected:
	virtual void BeginPlay() override;

public:
	/** Essential: Starting room template. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	TObjectPtr<class URoomTemplate> StartRoomTemplate;

	/** Essential: Boss room template. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	TObjectPtr<class URoomTemplate> BossRoomTemplate;

	/** Variety: List of combat-oriented rooms. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	TArray<TObjectPtr<class URoomTemplate>> CombatRoomTemplates;

	/** Variety: List of treasure/loot rooms. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	TArray<TObjectPtr<class URoomTemplate>> TreasureRoomTemplates;

	/** Variety: List of rest/safe rooms. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	TArray<TObjectPtr<class URoomTemplate>> RestRoomTemplates;

	/** Number of rooms to generate in the dungeon. */
	UPROPERTY(EditAnywhere, Category = "RoomGeneration")
	int32 RoomsToGenerate = 8;
};
