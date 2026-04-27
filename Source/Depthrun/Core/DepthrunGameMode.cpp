// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunGameMode.h"
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "Engine/GameInstance.h"
#include "DepthrunLogChannels.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"

ADepthrunGameMode::ADepthrunGameMode() {}

void ADepthrunGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UMusicSubsystem* Music = UGameplayStatics::GetGameInstance(GetWorld())
		->GetSubsystem<UMusicSubsystem>())
	{
		Music->PlayMusic(EMusicTrack::Explore, 1.5f, 1.5f);
		UE_LOG(LogDepthrunMusic, Log, TEXT("[GameMode] Playing Explore music"));
	}
	if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		Player->SetActorHiddenInGame(true);
		Player->SetActorEnableCollision(false);
		if (APaperCharacter* PaperPlayer = Cast<APaperCharacter>(Player))
		{
			if (UPaperFlipbookComponent* Sprite = PaperPlayer->GetSprite())
			{
				Sprite->SetHiddenInGame(true);
				Sprite->SetVisibility(false, true);
			}
		}
	}

	// Preload Blueprint classes (projectiles, VFX) to eliminate first-shot hitches.
	for (const TSoftClassPtr<AActor>& SoftClass : PreloadClasses)
	{
		if (!SoftClass.IsNull())
		{
			SoftClass.LoadSynchronous();
			UE_LOG(LogDepthrun, Log, TEXT("[GameMode] Preloaded class: %s"), *SoftClass.ToString());
		}
	}

	if (URoomGeneratorSubsystem* RoomGen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
	{
		RoomGen->SetTemplates(StartRoomTemplate, BossRoomTemplate, CombatRoomTemplates, TreasureRoomTemplates, RestRoomTemplates);
		RoomGen->GenerateRooms(RoomsToGenerate);
	}
}
