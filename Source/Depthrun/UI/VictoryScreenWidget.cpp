// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/VictoryScreenWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/UISoundLibrary.h"

void UVictoryScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToHubBtn)  ToHubBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnToHubClicked);
	if (ToMenuBtn) ToMenuBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnToMenuClicked);
	if (QuitBtn)   QuitBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnQuitClicked);

	auto BindHover = [this](UButton* Btn) {
		if (Btn) Btn->OnHovered.AddLambda([this]() {
			if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
				SFX->PlayButtonHover();
		});
	};
	BindHover(ToHubBtn); BindHover(ToMenuBtn); BindHover(QuitBtn);
}

void UVictoryScreenWidget::Show(float RunTimeSeconds, int32 EarnedDiamonds, int32 TotalDiamondsAfter)
{
	int32 Minutes = FMath::FloorToInt(RunTimeSeconds / 60.f);
	int32 Seconds = FMath::FloorToInt(RunTimeSeconds) % 60;

	if (RunTimeText)
		RunTimeText->SetText(FText::FromString(
			FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));

	if (EarnedDiamondsText)
	{
		FString DiamondStr = (EarnedDiamonds > 0)
			? FString::Printf(TEXT("+%d"), EarnedDiamonds)
			: TEXT("0");
		EarnedDiamondsText->SetText(FText::FromString(DiamondStr));
	}

	if (TotalDiamondsText)
		TotalDiamondsText->SetText(FText::FromString(
			FString::Printf(TEXT("%d"), TotalDiamondsAfter)));

	SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}

void UVictoryScreenWidget::PlayClickSound()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
}

void UVictoryScreenWidget::OnToHubClicked()
{
	PlayClickSound();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_Hub"));
}

void UVictoryScreenWidget::OnToMenuClicked()
{
	PlayClickSound();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_MainMenu"));
}

void UVictoryScreenWidget::OnQuitClicked()
{
	PlayClickSound();
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
