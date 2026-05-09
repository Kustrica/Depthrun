// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/VictoryScreenWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UI/UISoundLibrary.h"

void UVictoryScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToHubBtn)  ToHubBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnToHubClicked);
	if (ToMenuBtn) ToMenuBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnToMenuClicked);
	if (QuitBtn)   QuitBtn->OnClicked.AddDynamic(this, &UVictoryScreenWidget::OnQuitClicked);

	if (ToHubBtn)  ToHubBtn->OnHovered.AddDynamic(this, &UVictoryScreenWidget::OnButtonHovered);
	if (ToMenuBtn) ToMenuBtn->OnHovered.AddDynamic(this, &UVictoryScreenWidget::OnButtonHovered);
	if (QuitBtn)   QuitBtn->OnHovered.AddDynamic(this, &UVictoryScreenWidget::OnButtonHovered);
}

void UVictoryScreenWidget::Show(float RunTimeSeconds, int32 RoomsCleared, int32 TotalRooms, int32 EarnedDiamonds, int32 TotalDiamondsAfter)
{
	int32 Minutes = FMath::FloorToInt(RunTimeSeconds / 60.f);
	int32 Seconds = FMath::FloorToInt(RunTimeSeconds) % 60;

	if (RunTimeText)
		RunTimeText->SetText(FText::FromString(
			FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));

	if (RoomsText)
		RoomsText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), RoomsCleared, TotalRooms)));

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

	// Spawn victory VFX at player location
	if (NS_VictoryEffect)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		if (APawn* PlayerPawn = GetOwningPlayerPawn())
		{
			SpawnLoc = PlayerPawn->GetActorLocation();
		}
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), NS_VictoryEffect, SpawnLoc,
			FRotator::ZeroRotator, FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}

void UVictoryScreenWidget::OnButtonHovered()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonHover();
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
