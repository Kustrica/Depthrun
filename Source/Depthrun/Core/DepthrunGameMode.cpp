// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunGameMode.h"
#include "DepthrunLogChannels.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"

ADepthrunGameMode::ADepthrunGameMode() {}

void ADepthrunGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (URoomGeneratorSubsystem* RoomGen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
	{
		RoomGen->SetTemplates(StartRoomTemplate, BossRoomTemplate, CombatRoomTemplates, TreasureRoomTemplates, RestRoomTemplates);
		RoomGen->GenerateRooms(RoomsToGenerate);
	}
}
