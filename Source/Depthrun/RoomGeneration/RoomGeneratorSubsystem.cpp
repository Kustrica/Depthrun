// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomGeneratorSubsystem.h"
#include "RoomBase.h"
#include "RoomTemplate.h"
#include "DungeonTypes.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Player/DepthrunCharacter.h"
#include "UI/DepthrunHUD.h"
#include "UI/HUDOverlayWidget.h"
#include "GameFramework/PlayerController.h"

namespace RoomGeneratorLocal
{
constexpr float RoomGenBaseTileSize = 16.0f;

float ResolveRoomGenWorldScale(const URoomTemplate* Template)
{
    if (!Template || Template->WorldScale <= 0.01f)
    {
        return 2.6f;
    }
    return Template->WorldScale;
}

FVector ResolveRoomGenSpawnOffset(const URoomTemplate* Template)
{
    if (!Template)
    {
        return FVector::ZeroVector;
    }
    return Template->SpawnOffset;
}
} // namespace RoomGeneratorLocal

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

    // Branching walk: 70% chance to continue from the last room (creates arms),
    // 30% chance to branch from any existing room (avoids dead-ends on small counts).
    // This produces tree-like layouts with gaps between branches.
    int32 LastIdx = 0;
    for (int32 i = 1; i < RoomCount; ++i)
    {
        bool bFound = false;
        for (int32 At = 0; At < 40; ++At)
        {
            // Bias strongly toward extending the current tip of the walk
            FIntPoint Parent;
            if (FMath::FRand() < 0.7f)
                Parent = Coords[LastIdx];
            else
                Parent = Coords[FMath::RandRange(0, Coords.Num() - 1)];

            // Shuffle directions for variety
            TArray<FIntPoint> ShuffledDirs = Dirs;
            for (int32 d = ShuffledDirs.Num() - 1; d > 0; --d)
            {
                int32 j = FMath::RandRange(0, d);
                ShuffledDirs.Swap(d, j);
            }

            for (const FIntPoint& Dir : ShuffledDirs)
            {
                FIntPoint Candidate = Parent + Dir;
                // Reject cells that already have 2+ occupied neighbors (prevents clumping)
                if (Occupied.Contains(Candidate)) continue;
                int32 NeighborCount = 0;
                for (const FIntPoint& D2 : Dirs)
                    if (Occupied.Contains(Candidate + D2)) ++NeighborCount;
                if (NeighborCount > 1) continue; // keeps layout sparse

                Coords.Add(Candidate);
                Occupied.Add(Candidate);
                LastIdx = Coords.Num() - 1;
                bFound = true;
                break;
            }
            if (bFound) break;
        }
        if (!bFound)
        {
            // Fallback: relax neighbor constraint and pick any free adjacent cell
            for (int32 At2 = 0; At2 < 20 && !bFound; ++At2)
            {
                FIntPoint Parent = Coords[FMath::RandRange(0, Coords.Num() - 1)];
                FIntPoint Candidate = Parent + Dirs[FMath::RandRange(0, 3)];
                if (!Occupied.Contains(Candidate))
                {
                    Coords.Add(Candidate);
                    Occupied.Add(Candidate);
                    LastIdx = Coords.Num() - 1;
                    bFound = true;
                }
            }
        }
        if (!bFound) break;
    }

    // 2b. Boss Room = leaf node (1 neighbor) farthest from start.
    // Leaf nodes are branch tips — they can never appear "in the middle".
    // Fallback: if no leaf found beyond index 0, use farthest any room.
    if (Coords.Num() > 2)
    {
        int32 BossIdx = -1;
        int32 MaxDist = 0;

        // Pass 1: prefer leaf nodes (rooms with exactly 1 occupied neighbor)
        for (int32 k = 1; k < Coords.Num(); ++k)
        {
            int32 NeighborCount = 0;
            for (const FIntPoint& D : Dirs)
                if (Occupied.Contains(Coords[k] + D)) ++NeighborCount;

            if (NeighborCount == 1)
            {
                int32 Dist = FMath::Abs(Coords[k].X) + FMath::Abs(Coords[k].Y);
                if (Dist > MaxDist)
                {
                    MaxDist = Dist;
                    BossIdx = k;
                }
            }
        }

        // Pass 2 fallback: no suitable leaf — use plain farthest
        if (BossIdx == -1)
        {
            for (int32 k = 1; k < Coords.Num(); ++k)
            {
                int32 Dist = FMath::Abs(Coords[k].X) + FMath::Abs(Coords[k].Y);
                if (Dist > MaxDist) { MaxDist = Dist; BossIdx = k; }
            }
        }

        if (BossIdx != -1 && BossIdx != Coords.Num() - 1)
            Coords.Swap(BossIdx, Coords.Num() - 1);

        UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Boss Room at grid (%d,%d), dist=%d from start"),
               Coords.Last().X, Coords.Last().Y, MaxDist);
    }

    // 3. Расчет размеров с нахлестом 1 пиксель мира для устранения пиксельных зазоров.
    // Из-за поворота тайлмапа (-90, 0, 90):
    // Ось X в мире (Вверх/Вниз) соответствует высоте комнаты (6 тайлов).
    // Ось Y в мире (Влево/Вправо) соответствует ширине комнаты (8 тайлов).
    const float TileSize = RoomGeneratorLocal::RoomGenBaseTileSize *
                           RoomGeneratorLocal::ResolveRoomGenWorldScale(StartTemplate);
    const float RoomSizeX = 6.0f * TileSize; // World X (6 tile rows)
    const float RoomSizeY = 8.0f * TileSize; // World Y (8 tile cols)

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
            // Use PlayerLockedZ directly — no clamping, full DataAsset control.
            SpawnPos.Z = StartTemplate ? StartTemplate->PlayerLockedZ : 1.0f;
            Player->SetActorLocation(SpawnPos);
            Player->SetActorRotation(FRotator::ZeroRotator);
            Player->SetActorHiddenInGame(false);
            Player->SetActorEnableCollision(true);

            // Pin Z permanently — prevents the character climbing on door/obstacle
            // collision geometry due to Flying mode "step-up" behaviour.
            if (ADepthrunCharacter* DepthrunPlayer = Cast<ADepthrunCharacter>(Player))
            {
                const float LockedZ = StartTemplate ? StartTemplate->PlayerLockedZ : 4.0f;
                DepthrunPlayer->SetLockedZ(LockedZ);
                UE_LOG(LogTemp, Log, TEXT("[RoomGen] Player Z locked to %.1f (from template)"), LockedZ);
            }

            // Force-refresh the PaperCharacter sprite so the player is visible
            // immediately after teleport (without requiring a movement input).
            if (APaperCharacter* PaperPlayer = Cast<APaperCharacter>(Player))
            {
                if (UPaperFlipbookComponent* Sprite = PaperPlayer->GetSprite())
                {
                    Sprite->SetHiddenInGame(false);
                    Sprite->SetVisibility(true, true);
                    Sprite->SetTranslucentSortPriority(100); // Render player on top
                    Sprite->MarkRenderStateDirty();
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Successfully generated %d rooms"), GeneratedRooms.Num());
}

void URoomGeneratorSubsystem::ActivateRoom(int32 Index)
{
    if (!GeneratedRooms.IsValidIndex(Index)) return;
    
    CurrentRoomIndex = Index;
    
    // Start room (index 0) should not close doors - player needs to exit
    if (Index == 0)
    {
        GeneratedRooms[Index]->SetIsActive(true);
        UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Activating Start Room %d (doors stay open)"), Index);
    }
    else
    {
        GeneratedRooms[Index]->ActivateRoom();
        UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Activating Combat Room %d"), Index);
    }
}

void URoomGeneratorSubsystem::OnPlayerEnteredRoom(ARoomBase* Room)
{
    if (!Room) return;
    int32 Index = GeneratedRooms.IndexOfByKey(Room);
    if (Index != INDEX_NONE && Index != CurrentRoomIndex)
    {
        ActivateRoom(Index);
    }

    // Update HUD room counter (cleared rooms / total)
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        if (ADepthrunHUD* HUD = Cast<ADepthrunHUD>(PC->GetHUD()))
            if (UHUDOverlayWidget* Overlay = HUD->GetHUDOverlay())
                Overlay->SetRoomInfo(GetClearedRoomsCount(), GeneratedRooms.Num() - 1);
}

void URoomGeneratorSubsystem::SetTemplates(URoomTemplate* Start, URoomTemplate* Boss, const TArray<URoomTemplate*>& Combat, const TArray<URoomTemplate*>& Treasure, const TArray<URoomTemplate*>& Rest)
{
    StartTemplate = Start;
    BossTemplate = Boss;
    CombatPool = Combat;
    TreasurePool = Treasure;
    RestPool = Rest;
}

ARoomBase* URoomGeneratorSubsystem::GetCurrentActiveRoom() const
{
	if (GeneratedRooms.IsValidIndex(CurrentRoomIndex))
	{
		return GeneratedRooms[CurrentRoomIndex].Get();
	}
	return nullptr;
}

int32 URoomGeneratorSubsystem::GetClearedRoomsCount() const
{
    int32 Count = 0;
    // Skip index 0 (start room) — it auto-clears without combat and should not count
    for (int32 i = 1; i < GeneratedRooms.Num(); ++i)
    {
        if (GeneratedRooms[i] && GeneratedRooms[i]->IsCleared()) ++Count;
    }
    return Count;
}

void URoomGeneratorSubsystem::NotifyRoomCleared()
{
    // Update HUD immediately when a room is cleared
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        if (ADepthrunHUD* HUD = Cast<ADepthrunHUD>(PC->GetHUD()))
            if (UHUDOverlayWidget* Overlay = HUD->GetHUDOverlay())
                Overlay->SetRoomInfo(GetClearedRoomsCount(), GeneratedRooms.Num() - 1);
}

void URoomGeneratorSubsystem::OnPlayerEnteredTransition(ARoomBase* FromRoom, int32 ExitIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Player entered transition from room %s"), FromRoom ? *FromRoom->GetName() : TEXT("NULL"));
    
    // Open doors in the room the player is leaving
    if (FromRoom && !FromRoom->IsCleared())
    {
        // Only open doors if the room was cleared (enemies defeated)
        // Otherwise keep them closed to prevent backtracking during combat
        UE_LOG(LogTemp, Log, TEXT("[DungeonGen] Opening doors in room %s (player leaving)"), *FromRoom->GetName());
        FromRoom->DeactivateRoom();
    }
}
