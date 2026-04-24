// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunGameMode.h"
#include "DepthrunLogChannels.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"

ADepthrunGameMode::ADepthrunGameMode()
{
	// Default classes are assigned in Project Settings → Maps & Modes
}

void ADepthrunGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogDepthrun, Log, TEXT("ADepthrunGameMode::BeginPlay — session started"));

	// Initialize Room Generator Subsystem with templates from GameMode
	if (URoomGeneratorSubsystem* RoomGen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
	{
		RoomGen->SetTemplates(StartRoomTemplate, CombatRoomTemplates, BossRoomTemplate);
	}
}
