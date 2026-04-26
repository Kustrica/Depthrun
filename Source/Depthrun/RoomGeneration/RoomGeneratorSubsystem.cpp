// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomGeneratorSubsystem.h"
#include "RoomBase.h"
#include "RoomTemplate.h"
#include "DungeonTypes.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

namespace {
constexpr float BaseTileSize = 16.0f;
const FVector LegacySpawnOffset = FVector(-96.f, 144.f, 0.f);

float ResolveWorldScale(const URoomTemplate* Template)
{
    if (!Template || Template->WorldScale <= 0.01f)
    {
        return 2.6f;
    }
    return Template->WorldScale;
}

FVector ResolveSpawnOffset(const URoomTemplate* Template)
{
    if (!Template)
    {
        return FVector::ZeroVector;
    }

    if (Template->SpawnOffset.IsNearlyZero(0.01f))
    {
        return LegacySpawnOffset;
    }

    return Template->SpawnOffset;
}
} // namespace

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

    // 1. Очистка старых комнат
    for (ARoomBase* Room : GeneratedRooms)
    {
        if (Room) Room->Destroy();
    }
    GeneratedRooms.Empty();
    CurrentRoomIndex = 0;

    // 2. Генерация разметки (Random Walk)
    TSet<FIntPoint> Occupied;
    TArray<FIntPoint> Coords;
    FIntPoint Pos(0, 0);
    Coords.Add(Pos);
    Occupied.Add(Pos);

    TArray<FIntPoint> Dirs = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    for (int32 i = 1; i < RoomCount; ++i)
    {
        bool bFound = false;
        for (int32 At = 0; At < 30; ++At)
        {
            FIntPoint Parent = Coords[FMath::RandRange(0, Coords.Num() - 1)];
            FIntPoint Candidate = Parent + Dirs[FMath::RandRange(0, 3)];
            if (!Occupied.Contains(Candidate))
            {
                Coords.Add(Candidate);
                Occupied.Add(Candidate);
                bFound = true;
                break;
            }
        }
        if (!bFound) break;
    }

    // 3. Расчет размеров (БЕЗ нахлеста)
    // Из-за поворота тайлмапа (-90, 0, 90):
    // Ось X в мире (Вверх/Вниз) соответствует высоте комнаты (6 тайлов).
    // Ось Y в мире (Влево/Вправо) соответствует ширине комнаты (8 тайлов).
    const float TileSize = BaseTileSize * ResolveWorldScale(StartTemplate);
    const float RoomSizeX = 6.0f * TileSize; // World X
    const float RoomSizeY = 8.0f * TileSize; // World Y

    // 4. Спавн актеров
    for (int32 i = 0; i < Coords.Num(); ++i)
    {
        FIntPoint C = Coords[i];
        // C.X это колонка (Влево/Вправо -> World Y)
        // C.Y это строка (Вверх/Вниз -> World X)
        FVector Loc = FVector(C.Y * RoomSizeX, C.X * RoomSizeY, 0.f);

        URoomTemplate* T = nullptr;
        if (i == 0) T = StartTemplate;
        else if (i == Coords.Num() - 1) T = BossTemplate;
        else
        {
            float Rand = FMath::FRand();
            if (Rand < 0.15f && TreasurePool.Num() > 0) T = TreasurePool[FMath::RandRange(0, TreasurePool.Num() - 1)];
            else if (Rand < 0.25f && RestPool.Num() > 0) T = RestPool[FMath::RandRange(0, RestPool.Num() - 1)];
            else if (CombatPool.Num() > 0) T = CombatPool[FMath::RandRange(0, CombatPool.Num() - 1)];
            
            // Если пулы пусты, берем любую боевую
            if (!T && CombatPool.Num() > 0) T = CombatPool[0];
        }

        if (T)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ARoomBase* NewRoom = GetWorld()->SpawnActor<ARoomBase>(ARoomBase::StaticClass(), Loc, FRotator::ZeroRotator, Params);
            if (NewRoom)
            {
                // Для C.Y (World X, Up/Down): +1 это Top, -1 это Bottom
                // Для C.X (World Y, Left/Right): -1 это Left, +1 это Right
                NewRoom->SetupRoom(T, 
                    Occupied.Contains(C + FIntPoint(0, 1)),  // Top
                    Occupied.Contains(C + FIntPoint(0, -1)), // Bottom
                    Occupied.Contains(C + FIntPoint(-1, 0)), // Left
                    Occupied.Contains(C + FIntPoint(1, 0))); // Right
                GeneratedRooms.Add(NewRoom);
            }
        }
    }

    // 5. Инициализация игрока в первой комнате
    if (GeneratedRooms.Num() > 0)
    {
        ActivateRoom(0);
        if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
        {
            FVector SpawnPos = GeneratedRooms[0]->GetActorLocation();
            if (StartTemplate) {
                SpawnPos += ResolveSpawnOffset(StartTemplate);
                SpawnPos.Z = StartTemplate->PlayerSpawnZ;
            } else {
                SpawnPos.Z = 1.0f;
            }
            Player->SetActorLocation(SpawnPos);
            Player->SetActorRotation(FRotator::ZeroRotator);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Successfully generated %d rooms"), GeneratedRooms.Num());
}

void URoomGeneratorSubsystem::ActivateRoom(int32 Index)
{
    if (!GeneratedRooms.IsValidIndex(Index)) return;
    
    CurrentRoomIndex = Index;
    GeneratedRooms[Index]->ActivateRoom();
    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Activating Room %d"), Index);
}

void URoomGeneratorSubsystem::OnPlayerEnteredRoom(ARoomBase* Room)
{
    if (!Room) return;
    int32 Index = GeneratedRooms.IndexOfByKey(Room);
    if (Index != INDEX_NONE && Index != CurrentRoomIndex)
    {
        ActivateRoom(Index);
    }
}

void URoomGeneratorSubsystem::SetTemplates(URoomTemplate* Start, URoomTemplate* Boss, const TArray<URoomTemplate*>& Combat, const TArray<URoomTemplate*>& Treasure, const TArray<URoomTemplate*>& Rest)
{
    StartTemplate = Start;
    BossTemplate = Boss;
    CombatPool = Combat;
    TreasurePool = Treasure;
    RestPool = Rest;
}

void URoomGeneratorSubsystem::OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex)
{
    // Logic to be implemented if needed for complex transitions
    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Player entered transition from room %s"), FromRoom ? *FromRoom->GetName() : TEXT("NULL"));
}
