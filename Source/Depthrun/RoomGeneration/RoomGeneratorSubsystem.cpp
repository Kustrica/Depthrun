// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomGeneratorSubsystem.h"
#include "DepthrunLogChannels.h"

void URoomGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection) { Super::Initialize(Collection); }
void URoomGeneratorSubsystem::Deinitialize()                                    { Super::Deinitialize(); }

void URoomGeneratorSubsystem::GenerateRooms(int32 RoomCount)
{
	UE_LOG(LogRoomGen, Log, TEXT("[RoomGen] GenerateRooms(%d) — stub"), RoomCount);
	// TODO (Stage 8B): implement BSP/Random Walk, spawn actors
}

void URoomGeneratorSubsystem::OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex)
{
	// TODO (Stage 8B): advance CurrentRoomIndex, activate next room
}

void URoomGeneratorSubsystem::SpawnRoom(URoomTemplate* Template, const FVector& Location)
{
	// TODO (Stage 8B): SpawnActor<ARoomBase> with template class
}

void URoomGeneratorSubsystem::ActivateRoom(int32 Index)
{
	if (!GeneratedRooms.IsValidIndex(Index)) return;
	if (ARoomBase* Room = GeneratedRooms[Index]) { Room->ActivateRoom(); }
	CurrentRoomIndex = Index;
}
