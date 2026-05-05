// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VictoryScreenWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * UVictoryScreenWidget
 * Shows run stats on win. Buttons: Hub / Main Menu / Quit.
 * Call Show(...) from C++ when boss room is cleared.
 */
UCLASS()
class DEPTHRUN_API UVictoryScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	/**
	 * Populate stats and show the widget.
	 * @param RunTimeSeconds      Total run time in seconds.
	 * @param EarnedDiamonds      Diamonds earned this run.
	 * @param TotalDiamondsAfter  Profile total after adding earned.
	 */
	void Show(float RunTimeSeconds, int32 RoomsCleared, int32 TotalRooms, int32 EarnedDiamonds, int32 TotalDiamondsAfter);

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
};
