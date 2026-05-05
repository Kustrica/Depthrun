// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "ChestRewardWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UImage;

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

	// ─── Timing ────────────────────────────────────────────────────────────
	/** How long the popup stays open before auto-close. Change in WBP defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Timing")
	float AutoCloseDelay = 5.f;

	// ─── Icons ─────────────────────────────────────────────────────────────
	/** Size of each reward icon in pixels (width = height). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Icons",
		meta = (ClampMin = "16", ClampMax = "128"))
	float IconSize = 32.f;

	// ─── Typography ────────────────────────────────────────────────────────
	/** Font for reward lines (item name, diamond count, potion count). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Typography")
	FSlateFontInfo RewardFont;

	/** Font size for reward lines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Typography",
		meta = (ClampMin = "8", ClampMax = "72"))
	int32 RewardFontSize = 20;

	/** Color of reward line text. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Typography")
	FLinearColor RewardTextColor = FLinearColor::White;

	/** Font size for the countdown text. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Typography",
		meta = (ClampMin = "8", ClampMax = "48"))
	int32 CountdownFontSize = 14;

	/** Color of the countdown text. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChestReward|Typography")
	FLinearColor CountdownTextColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.f);

	// ─── BindWidget — names must match exactly in WBP ──────────────────────

	/** Full-screen scroll PNG background. Set your scroll texture in WBP. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ScrollBackground;

	/** Vertical box inside the scroll where reward rows are added. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RewardLinesBox;

	/** Countdown text at the bottom of the scroll. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AutoCloseText;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

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
