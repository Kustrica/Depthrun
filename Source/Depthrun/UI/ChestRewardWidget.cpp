// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/ChestRewardWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "UI/UISoundLibrary.h"
#include "Core/DepthrunLogChannels.h"

void UChestRewardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ClickOverlay)
		ClickOverlay->OnClicked.AddDynamic(this, &UChestRewardWidget::OnAutoClose);
}

void UChestRewardWidget::Show(int32 Diamonds, int32 Potions, const FString& ItemName,
	UTexture2D* DiamondIcon, UTexture2D* PotionIcon, UTexture2D* ItemIcon)
{
	// Play chest open SFX
	if (UGameInstance* GI = GetGameInstance())
		if (UUISoundLibrary* SFX = GI->GetSubsystem<UUISoundLibrary>())
			SFX->PlayChestOpen();

	// Clear previous rows
	if (RewardLinesBox)
		RewardLinesBox->ClearChildren();

	// Diamonds row
	if (Diamonds > 0)
		AddRewardRow(DiamondIcon, FString::Printf(TEXT("+%d diamonds"), Diamonds));

	// Potions row
	if (Potions > 0)
		AddRewardRow(PotionIcon, FString::Printf(TEXT("+%d potions"), Potions));

	// Item row
	if (!ItemName.IsEmpty())
		AddRewardRow(ItemIcon, ItemName);

	// Add to viewport
	AddToViewport(20);

	// Start countdown
	SecondsRemaining = FMath::RoundToInt(AutoCloseDelay);
	if (AutoCloseText)
	{
		AutoCloseText->SetText(FText::FromString(
			FString::Printf(TEXT("Closing in %d..."), SecondsRemaining)));

		FSlateFontInfo CFont = AutoCloseText->GetFont();
		CFont.Size = CountdownFontSize;
		AutoCloseText->SetFont(CFont);
		AutoCloseText->SetColorAndOpacity(FSlateColor(CountdownTextColor));
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(TimerHandle_Countdown, this,
			&UChestRewardWidget::OnCountdownTick, 1.f, true);
		World->GetTimerManager().SetTimer(TimerHandle_AutoClose, this,
			&UChestRewardWidget::OnAutoClose, AutoCloseDelay, false);
	}

	UE_LOG(LogDepthrunLoot, Log, TEXT("[ChestRewardWidget] Showing reward popup: %d diamonds, %d potions, item='%s'"),
		Diamonds, Potions, *ItemName);
}

void UChestRewardWidget::AddRewardRow(UTexture2D* Icon, const FString& Label)
{
	if (!RewardLinesBox) return;

	UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

	// Icon — size driven by IconSize property
	if (IsValid(Icon))
	{
		UImage* IconWidget = NewObject<UImage>(this);
		FSlateBrush Brush;
		Brush.SetResourceObject(Icon);
		Brush.ImageSize = FVector2D(IconSize, IconSize);
		IconWidget->SetBrush(Brush);

		UHorizontalBoxSlot* IconSlot = Row->AddChildToHorizontalBox(IconWidget);
		IconSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		IconSlot->SetVerticalAlignment(VAlign_Center);
		IconSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
	}

	// Text label — font/size/color from configurable properties
	UTextBlock* TextWidget = NewObject<UTextBlock>(this);
	TextWidget->SetText(FText::FromString(Label));

	FSlateFontInfo Font = RewardFont.HasValidFont() ? RewardFont : TextWidget->GetFont();
	Font.Size = RewardFontSize;
	TextWidget->SetFont(Font);
	TextWidget->SetColorAndOpacity(FSlateColor(RewardTextColor));

	UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(TextWidget);
	TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	TextSlot->SetVerticalAlignment(VAlign_Center);

	UVerticalBoxSlot* RowSlot = RewardLinesBox->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.f, 4.f));
	RowSlot->SetHorizontalAlignment(HAlign_Left);
}

void UChestRewardWidget::OnCountdownTick()
{
	--SecondsRemaining;
	if (AutoCloseText && SecondsRemaining > 0)
		AutoCloseText->SetText(FText::FromString(
			FString::Printf(TEXT("Closing in %d..."), SecondsRemaining)));
}

void UChestRewardWidget::OnAutoClose()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_Countdown);
		World->GetTimerManager().ClearTimer(TimerHandle_AutoClose);
	}
	RemoveFromParent();
}
