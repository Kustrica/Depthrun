// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/DeathScreenWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UI/UISoundLibrary.h"

void UDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToHubBtn)  ToHubBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnToHubClicked);
	if (ToMenuBtn) ToMenuBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnToMenuClicked);
	if (QuitBtn)   QuitBtn->OnClicked.AddDynamic(this, &UDeathScreenWidget::OnQuitClicked);

	if (ToHubBtn)  ToHubBtn->OnHovered.AddDynamic(this, &UDeathScreenWidget::OnButtonHovered);
	if (ToMenuBtn) ToMenuBtn->OnHovered.AddDynamic(this, &UDeathScreenWidget::OnButtonHovered);
	if (QuitBtn)   QuitBtn->OnHovered.AddDynamic(this, &UDeathScreenWidget::OnButtonHovered);
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
	int32 DiamondsFinal = EarnedDiamonds - DiamondsPenalty;
	if (EarnedDiamondsText)
	{
		FString DiamondStr = (EarnedDiamonds > 0)
			? FString::Printf(TEXT("+%d  (-%d penalty  = +%d)"), EarnedDiamonds, DiamondsPenalty, DiamondsFinal)
			: TEXT("0");
		EarnedDiamondsText->SetText(FText::FromString(DiamondStr));
	}

	if (TotalDiamondsText)
		TotalDiamondsText->SetText(FText::FromString(
			FString::Printf(TEXT("%d"), TotalDiamondsAfter)));

	SetVisibility(ESlateVisibility::Visible);

	// Spawn death VFX at player location
	if (NS_DeathEffect)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		if (APawn* PlayerPawn = GetOwningPlayerPawn())
		{
			SpawnLoc = PlayerPawn->GetActorLocation();
		}
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), NS_DeathEffect, SpawnLoc,
			FRotator::ZeroRotator, FVector(1.f), true, true,
			ENCPoolMethod::AutoRelease);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}

void UDeathScreenWidget::OnButtonHovered()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonHover();
}

void UDeathScreenWidget::PlayClickSound()
{
	if (UUISoundLibrary* SFX = GetGameInstance()->GetSubsystem<UUISoundLibrary>())
		SFX->PlayButtonClick();
}

void UDeathScreenWidget::OnToHubClicked()
{
	PlayClickSound();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_Hub"));
}

void UDeathScreenWidget::OnToMenuClicked()
{
	PlayClickSound();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_MainMenu"));
}

void UDeathScreenWidget::OnQuitClicked()
{
	PlayClickSound();
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}
