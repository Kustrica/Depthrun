// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UWidget;
class USettingsWidget;

/**
 * UPauseMenuWidget
 * Pause overlay. Bind buttons by name in WBP (BindWidget).
 * C++ handles all logic — Blueprint does layout only.
 */
UCLASS()
class DEPTHRUN_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	/** Called from DepthrunCharacter::TogglePause — sets the widget visible and pauses. */
	void Show();

	/** Assign WBP_Settings in BP_PauseMenu Class Defaults. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

	// ── Buttons — name MUST match exactly in WBP (BindWidget) ────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToHubBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToMenuBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitBtn;

private:
	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnToHubClicked();

	UFUNCTION()
	void OnToMenuClicked();

	UFUNCTION()
	void OnQuitClicked();
};
