// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "MainMenuWidget.generated.h"

class UButton;

/**
 * UMainMenuWidget
 * First screen the player sees on launch.
 * Flow: Main Menu → [Play] → Hub/Lobby level load → [Start Run] → gameplay level
 *
 * Layout done in UMG Editor (WBP_MainMenuWidget).
 * Implementation: Stage 9A.
 */
class USettingsWidget;

UCLASS()
class DEPTHRUN_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by Play button click. Loads the Hub level. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnPlayPressed();

	/** Called by Settings button click. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnSettingsPressed();

	/** Called by Quit button click. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnQuitPressed();

	/** Called by any button On Hovered event. */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnButtonHovered();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Override in Blueprint: animate the transition. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void PlayTransitionOut();

	/** Level name for the Hub. Set in GameInstance or hardcoded here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu")
	FName HubLevelName = TEXT("L_Hub");

	/** Assign WBP_Settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

	// ── Buttons — names must match exactly in WBP (BindWidget) ───────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PlayBtn;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsBtn;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitBtn;
};
