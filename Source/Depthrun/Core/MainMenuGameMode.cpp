// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Core/MainMenuGameMode.h"
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DepthrunLogChannels.h"

AMainMenuGameMode::AMainMenuGameMode()
{
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UMusicSubsystem* Music = UGameplayStatics::GetGameInstance(GetWorld())
		->GetSubsystem<UMusicSubsystem>())
	{
		if (Music->GetCurrentTrack() != EMusicTrack::Hub)
		{
			Music->PlayMusic(EMusicTrack::Hub, 1.5f, 1.0f);
			UE_LOG(LogDepthrunMusic, Log, TEXT("[MainMenuGameMode] Starting Hub music"));
		}
		else
		{
			UE_LOG(LogDepthrunMusic, Log, TEXT("[MainMenuGameMode] Hub music already playing — skip"));
		}
	}

	if (MainMenuWidgetClass && GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				PC->bShowMouseCursor = true;
				PC->SetInputMode(FInputModeUIOnly());
				UE_LOG(LogDepthrun, Log, TEXT("[MainMenuGameMode] Main menu widget added to viewport"));
			}
		}
	}
}
