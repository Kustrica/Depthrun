// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunGameMode.h"
#include "DepthrunLogChannels.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"

ADepthrunGameMode::ADepthrunGameMode() {}

void ADepthrunGameMode::BeginPlay()
{
	Super::BeginPlay();
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

	if (URoomGeneratorSubsystem* RoomGen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
	{
		RoomGen->SetTemplates(StartRoomTemplate, BossRoomTemplate, CombatRoomTemplates, TreasureRoomTemplates, RestRoomTemplates);
		RoomGen->GenerateRooms(RoomsToGenerate);
	}
}
