// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Core/HubGameMode.h"
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DepthrunLogChannels.h"

AHubGameMode::AHubGameMode()
{
}

void AHubGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UMusicSubsystem* Music = UGameplayStatics::GetGameInstance(GetWorld())
		->GetSubsystem<UMusicSubsystem>())
	{
		if (Music->GetCurrentTrack() != EMusicTrack::Hub)
		{
			Music->PlayMusic(EMusicTrack::Hub, 1.5f, 1.5f);
			UE_LOG(LogDepthrunMusic, Log, TEXT("[HubGameMode] Starting Hub music"));
		}
		else
		{
			UE_LOG(LogDepthrunMusic, Log, TEXT("[HubGameMode] Hub music already playing — skip"));
		}
	}

	if (HubWidgetClass && GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, HubWidgetClass);
			if (Widget)
			{
				Widget->AddToViewport();
				PC->bShowMouseCursor = true;
				PC->SetInputMode(FInputModeUIOnly());
				UE_LOG(LogDepthrun, Log, TEXT("[HubGameMode] Hub widget added to viewport"));
			}
		}
	}
}
