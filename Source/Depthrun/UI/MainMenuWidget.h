// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 * UMainMenuWidget
 * First screen the player sees on launch.
 * Flow: Main Menu → [Play] → Hub/Lobby level load → [Start Run] → gameplay level
 *
 * Layout done in UMG Editor (WBP_MainMenuWidget).
 * Implementation: Stage 9A.
 */
UCLASS()
class DEPTHRUN_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by Play button click. Loads the Hub level. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnPlayPressed();

	/** Called by Quit button click. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnQuitPressed();

protected:
	/** Override in Blueprint: animate the transition. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void PlayTransitionOut();

	/** Level name for the Hub. Set in GameInstance or hardcoded here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu")
	FName HubLevelName = TEXT("L_Hub");
};
