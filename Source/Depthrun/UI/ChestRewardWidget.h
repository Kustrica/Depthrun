// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "ChestRewardWidget.generated.h"

class UVerticalBox;
class UTextBlock;

/**
 * UChestRewardWidget
 * Popup scroll shown when player opens a chest.
 * Dynamically populates reward rows (icon + text) and auto-closes after 3s.
 *
 * Blueprint child: WBP_ChestRewardScroll
 * Required BindWidget names: RewardLinesBox, AutoCloseText
 */
UCLASS()
class DEPTHRUN_API UChestRewardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Show the popup with chest rewards.
	 * @param Diamonds   Number of diamonds awarded (0 = skip row)
	 * @param Potions    Number of potions awarded (0 = skip row)
	 * @param ItemName   Run item name (empty = no item dropped)
	 * @param DiamondIcon  Icon texture for diamonds (can be null)
	 * @param PotionIcon   Icon texture for potions (can be null)
	 * @param ItemIcon     Icon texture for run item (can be null)
	 */
	UFUNCTION(BlueprintCallable, Category = "ChestReward")
	void Show(int32 Diamonds, int32 Potions, const FString& ItemName,
		UTexture2D* DiamondIcon, UTexture2D* PotionIcon, UTexture2D* ItemIcon);

	/** Auto-close countdown seconds. Default 3. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ChestReward")
	float AutoCloseDelay = 3.f;

	// ─── BindWidget — names must match exactly in WBP ──────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RewardLinesBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AutoCloseText;

protected:
	virtual void NativeConstruct() override;

private:
	/** Adds one reward row: [icon 32x32] [text]. Icon may be null → shows no image. */
	void AddRewardRow(UTexture2D* Icon, const FString& Label);

	UFUNCTION()
	void OnAutoClose();

	FTimerHandle TimerHandle_AutoClose;
	int32 SecondsRemaining = 3;

	FTimerHandle TimerHandle_Countdown;
	UFUNCTION()
	void OnCountdownTick();
};
