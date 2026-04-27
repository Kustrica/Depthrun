// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HubGameMode.generated.h"

/**
 * AHubGameMode
 * GameMode for the L_Hub level (between runs).
 * On BeginPlay: plays Hub music, spawns HubWidget.
 */
UCLASS()
class DEPTHRUN_API AHubGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHubGameMode();

protected:
	virtual void BeginPlay() override;

public:
	/** Widget class to spawn as the Hub UI. Assign BP_HubGameMode defaults → WBP_HubWidget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub|UI")
	TSubclassOf<UUserWidget> HubWidgetClass;
};
