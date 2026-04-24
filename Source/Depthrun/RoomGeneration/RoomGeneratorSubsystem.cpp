// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomGeneratorSubsystem.h"
#include "RoomBase.h"
#include "RoomTemplate.h"
#include "DungeonTypes.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void URoomGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void URoomGeneratorSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void URoomGeneratorSubsystem::GenerateRooms(int32 RoomCount)
{
    if (RoomCount <= 0) return;

    // 1. Cleanup existing rooms
    for (ARoomBase* Room : GeneratedRooms)
    {
        if (Room) Room->Destroy();
    }
    GeneratedRooms.Empty();

    // 2. Simple Random Walk Algorithm
    // Grid coordinate (0,0) is starting room
    TSet<FIntPoint> OccupiedCells;
    TArray<FIntPoint> RoomCoords;
    
    FIntPoint CurrentPos(0, 0);
    RoomCoords.Add(CurrentPos);
    OccupiedCells.Add(CurrentPos);

    TArray<FIntPoint> Directions = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    for (int32 i = 1; i < RoomCount; ++i)
    {
        bool bFoundSpot = false;
        // Try to branch from ANY existing room to make it non-linear
        for (int32 Attempt = 0; Attempt < 10; ++Attempt)
        {
            FIntPoint BranchFrom = RoomCoords[FMath::RandRange(0, RoomCoords.Num() - 1)];
            FIntPoint Dir = Directions[FMath::RandRange(0, 3)];
            FIntPoint Candidate = BranchFrom + Dir;

            if (!OccupiedCells.Contains(Candidate))
            {
                CurrentPos = Candidate;
                RoomCoords.Add(CurrentPos);
                OccupiedCells.Add(CurrentPos);
                bFoundSpot = true;
                break;
            }
        }
        if (!bFoundSpot) break; // Grid full or isolated
    }

    // 3. Spawn Actors
    // Room dimensions (8x6 tiles * 41.6 units/tile)
    const float TileSize = 16.0f * 2.6f;
    const float RoomWidth = 8.0f * TileSize;
    const float RoomHeight = 6.0f * TileSize;

    for (int32 i = 0; i < RoomCoords.Num(); ++i)
    {
        FIntPoint Coord = RoomCoords[i];
        FVector WorldPos = FVector(Coord.X * RoomWidth, Coord.Y * RoomHeight, 0.f);

        // Pick template: 0 is Start, last is Boss, rest is Combat
        URoomTemplate* SelectedTemplate = nullptr;
        if (i == 0) {
            // Use a combat template for start for now or a dedicated one
            SelectedTemplate = CombatRoomTemplates.Num() > 0 ? CombatRoomTemplates[0] : nullptr;
        }
        else if (i == RoomCoords.Num() - 1) {
            SelectedTemplate = BossRoomTemplate;
        }
        else {
            if (CombatRoomTemplates.Num() > 0)
                SelectedTemplate = CombatRoomTemplates[FMath::RandRange(0, CombatRoomTemplates.Num() - 1)];
        }

        if (SelectedTemplate)
        {
            // Determine connections (doors)
            bool bHasTop = OccupiedCells.Contains(Coord + FIntPoint(0, -1));
            bool bHasBottom = OccupiedCells.Contains(Coord + FIntPoint(0, 1));
            bool bHasLeft = OccupiedCells.Contains(Coord + FIntPoint(-1, 0));
            bool bHasRight = OccupiedCells.Contains(Coord + FIntPoint(1, 0));

            // Spawn ARoomBase at WorldPos
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ARoomBase* NewRoom = GetWorld()->SpawnActor<ARoomBase>(ARoomBase::StaticClass(), WorldPos, FRotator::ZeroRotator, Params);
            
            if (NewRoom)
            {
                NewRoom->SetupRoom(SelectedTemplate, bHasTop, bHasBottom, bHasLeft, bHasRight);
                GeneratedRooms.Add(NewRoom);
            }
        }
    }

    // 4. Start first room
    if (GeneratedRooms.Num() > 0)
    {
        ActivateRoom(0);
        
        // Move player to first room
        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (Player)
        {
            Player->SetActorLocation(GeneratedRooms[0]->GetActorLocation() + FVector(0, 0, 100.f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Generated %d rooms"), GeneratedRooms.Num());
}

void URoomGeneratorSubsystem::ActivateRoom(int32 Index)
{
    if (!GeneratedRooms.IsValidIndex(Index)) return;
    CurrentRoomIndex = Index;
    GeneratedRooms[Index]->ActivateRoom();
}

void URoomGeneratorSubsystem::OnPlayerEnteredRoom(ARoomBase* Room)
{
    if (!Room) return;

    int32 Index = GeneratedRooms.IndexOfByKey(Room);
    if (Index != INDEX_NONE && Index != CurrentRoomIndex)
    {
        CurrentRoomIndex = Index;
        Room->ActivateRoom();
        UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Player entered Room %d"), Index);
    }
}

void URoomGeneratorSubsystem::OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex)
{
    // Logic to find neighbor and activate it
}
