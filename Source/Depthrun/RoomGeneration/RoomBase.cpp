// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "DepthrunLogChannels.h"

ARoomBase::ARoomBase()
{
	PrimaryActorTick.bCanEverTick = false;
	RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
	RootComponent = RoomBounds;
}

void ARoomBase::BeginPlay() { Super::BeginPlay(); }

void ARoomBase::ActivateRoom()
{
	UE_LOG(LogRoomGen, Log, TEXT("[Room] ActivateRoom — stub"));
	// TODO (Stage 8A): spawn enemies at SpawnPoints, close door actors
}

void ARoomBase::DeactivateRoom()
{
	UE_LOG(LogRoomGen, Log, TEXT("[Room] DeactivateRoom — stub"));
	// TODO (Stage 8A): open door actors
}

bool ARoomBase::IsCleared() const
{
	// TODO (Stage 8A): check all SpawnedEnemies are destroyed
	return false;
}
