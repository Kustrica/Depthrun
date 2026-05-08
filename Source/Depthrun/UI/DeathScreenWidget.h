// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * UDeathScreenWidget
 * Shows run stats on death. Buttons: Hub / Main Menu / Quit.
 * Call Show(RunTime, RoomsCleared, TotalRooms, EarnedDiamonds, TotalDiamonds) from C++.
 */
UCLASS()
class DEPTHRUN_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	/** Populate stats and make the widget visible. */
	void Show(float RunTimeSeconds, int32 RoomsCleared, int32 TotalRooms,
	          int32 EarnedDiamonds, int32 TotalDiamondsAfter);

	// ── Stats labels ─────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RunTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoomsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EarnedDiamondsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TotalDiamondsText;

	// ── Buttons ───────────────────────────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToHubBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToMenuBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitBtn;

private:
	void PlayClickSound();
	UFUNCTION() void OnToHubClicked();
	UFUNCTION() void OnToMenuClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnButtonHovered();
};
