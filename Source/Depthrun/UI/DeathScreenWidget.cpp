// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/DeathScreenWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToHubBtn)  ToHubBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnToHubClicked);
	if (ToMenuBtn) ToMenuBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnToMenuClicked);
	if (QuitBtn)   QuitBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnQuitClicked);
}

void UDeathScreenWidget::Show(float RunTimeSeconds, int32 RoomsCleared, int32 TotalRooms,
                               int32 EarnedDiamonds, int32 TotalDiamondsAfter)
{
	// Format run time as M:SS
	int32 Minutes = FMath::FloorToInt(RunTimeSeconds / 60.f);
	int32 Seconds = FMath::FloorToInt(RunTimeSeconds) % 60;

	if (RunTimeText)
		RunTimeText->SetText(FText::FromString(
			FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));

	if (RoomsText)
		RoomsText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), RoomsCleared, TotalRooms)));

	int32 DiamondsPenalty = FMath::FloorToInt(EarnedDiamonds * 0.5f);
	if (EarnedDiamondsText)
		EarnedDiamondsText->SetText(FText::FromString(
			FString::Printf(TEXT("+%d (-%d penalty = +%d)"),
				EarnedDiamonds, DiamondsPenalty, EarnedDiamonds - DiamondsPenalty)));

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

void UDeathScreenWidget::OnToHubClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_Hub"));
}

void UDeathScreenWidget::OnToMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_MainMenu"));
}

void UDeathScreenWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
