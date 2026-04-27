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
		Music->PlayMusic(EMusicTrack::Hub, 0.f, 0.f);
		UE_LOG(LogDepthrunMusic, Log, TEXT("[MainMenuGameMode] Playing Hub music (instant)"));
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
