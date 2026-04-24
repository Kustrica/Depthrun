// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RoomBase.h"
#include "RoomTemplate.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "PaperTileLayer.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/BaseEnemy.h"
#include "DoorActor.h"
#include "RoomGeneratorSubsystem.h"
#include "TimerManager.h"

ARoomBase::ARoomBase()
{
    PrimaryActorTick.bCanEverTick = false;

    RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
    RootComponent = RoomBounds;
    RoomBounds->SetBoxExtent(FVector(150.f, 120.f, 100.f)); 
    RoomBounds->SetCollisionProfileName(TEXT("Trigger"));
    RoomBounds->OnComponentBeginOverlap.AddDynamic(this, &ARoomBase::OnRoomEntry);

    TileMapComponent = CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("TileMapComponent"));
    TileMapComponent->SetupAttachment(RootComponent);
    TileMapComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    TileMapComponent->SetRelativeScale3D(FVector(2.6f, 2.6f, 1.0f));
}

void ARoomBase::BeginPlay()
{
    Super::BeginPlay();
}

void ARoomBase::SetupRoom(URoomTemplate* Template, bool bHasTop, bool bHasBottom, bool bHasLeft, bool bHasRight)
{
    if (!Template || !Template->TileMapAsset) return;
    MyTemplate = Template;

    TileMapComponent->SetTileMap(Template->TileMapAsset);
    TileMapComponent->MakeTileMapEditable();

    // GRID CONFIG: 8x6 (X: 0 to 7, Y: 0 to 5)
    GenerateProps(bHasTop, bHasBottom, bHasLeft, bHasRight);
}

void ARoomBase::GenerateProps(bool bHasTop, bool bHasBottom, bool bHasLeft, bool bHasRight)
{
    if (!MyTemplate) return;

    const float TileSize = 16.0f * 2.6f;
    const FVector RoomOrigin = GetActorLocation();

    auto HandleDoorSide = [&](bool bExists, int32 X1, int32 Y1, int32 X2, int32 Y2, FVector SpawnOffset, FRotator Rot)
    {
        if (!bExists)
        {
            // Block passage with wall on Layer 2
            SetTileInLayer(2, X1, Y1, MyTemplate->WallTile);
            SetTileInLayer(2, X2, Y2, MyTemplate->WallTile);
        }
        else
        {
            // Place shadowed floor on Layer 0
            SetTileInLayer(0, X1, Y1, MyTemplate->DoorFloorTile);
            SetTileInLayer(0, X2, Y2, MyTemplate->DoorFloorTile);
            
            // Clear Door_Layer (1) and Walls (2) at this spot
            SetTileInLayer(1, X1, Y1, FRoomTileInfo());
            SetTileInLayer(1, X2, Y2, FRoomTileInfo());
            SetTileInLayer(2, X1, Y1, FRoomTileInfo());
            SetTileInLayer(2, X2, Y2, FRoomTileInfo());
            
            if (MyTemplate->DoorClass)
            {
                FVector SpawnLoc = RoomOrigin + SpawnOffset;
                AActor* Door = GetWorld()->SpawnActor<AActor>(MyTemplate->DoorClass, SpawnLoc, Rot);
                if (Door) 
                {
                    Door->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                    SpawnedDoors.Add(Door);
                }
            }
        }
    };

    HandleDoorSide(bHasTop, 3, 0, 4, 0, FVector(0.f, -TileSize * 3.f, 0.f), FRotator::ZeroRotator);
    HandleDoorSide(bHasBottom, 3, 5, 4, 5, FVector(0.f, TileSize * 3.f, 0.f), FRotator::ZeroRotator);
    HandleDoorSide(bHasLeft, 0, 2, 0, 3, FVector(-TileSize * 4.f, 0.f, 0.f), FRotator(0, 90, 0));
    HandleDoorSide(bHasRight, 7, 2, 7, 3, FVector(TileSize * 4.f, 0.f, 0.f), FRotator(0, 90, 0));

    // Torches
    TArray<FIntPoint> TorchSpots = { {1,0}, {2,0}, {5,0}, {6,0} };
    for (const FIntPoint& Spot : TorchSpots)
    {
        if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->TorchSpawnChance)
        {
            if (MyTemplate->TorchClass)
            {
                FVector TorchLoc = RoomOrigin + FVector((Spot.X - 3.5f) * TileSize, (Spot.Y - 2.5f) * TileSize, 5.0f);
                AActor* Torch = GetWorld()->SpawnActor<AActor>(MyTemplate->TorchClass, TorchLoc, FRotator::ZeroRotator);
                if (Torch) Torch->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
    }
}

void ARoomBase::OnRoomEntry(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (bIsCleared || !Pawn || !Pawn->IsPlayerControlled()) return;

    if (URoomGeneratorSubsystem* Sub = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
    {
        Sub->OnPlayerEnteredRoom(this);
    }
}

void ARoomBase::ActivateRoom()
{
    if (bIsCleared) return;
    
    SpawnEnemies();
    
    // Close all doors
    for (AActor* Door : SpawnedDoors)
    {
        if (ADoorActor* DA = Cast<ADoorActor>(Door)) DA->CloseDoor();
    }

    // Start checking for enemy status every 1 second
    GetWorldTimerManager().SetTimer(TimerHandle_CheckEnemies, this, &ARoomBase::CheckEnemiesStatus, 1.0f, true);
}

void ARoomBase::CheckEnemiesStatus()
{
    if (bIsCleared) return;

    // Clean up null/dead pointers
    SpawnedEnemies.RemoveAll([](const TWeakObjectPtr<AActor>& Enemy) {
        return !Enemy.IsValid() || Enemy->IsPendingKillPending();
    });

    if (SpawnedEnemies.Num() == 0)
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_CheckEnemies);
        DeactivateRoom();
    }
}

void ARoomBase::DeactivateRoom()
{
    if (bIsCleared) return;
    bIsCleared = true;
    
    for (AActor* Door : SpawnedDoors)
    {
        if (ADoorActor* DA = Cast<ADoorActor>(Door)) DA->OpenDoor();
    }
    
    if (MyTemplate && MyTemplate->RoomType == ERoomType::Boss)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DungeonGen] BOSS CLEARED! EXIT HATCH OPEN."));
    }
    
    TrySpawnChest();
}

void ARoomBase::SpawnEnemies()
{
    if (!MyTemplate || MyTemplate->RoomType == ERoomType::Start) return;
    
    int32 Count = FMath::RandRange(MyTemplate->MinEnemies, MyTemplate->MaxEnemies);
    const float TileSize = 16.0f * 2.6f;

    for (int32 i = 0; i < Count; ++i)
    {
        if (MyTemplate->PotentialEnemies.Num() == 0) break;
        
        int32 RX = FMath::RandRange(1, 6);
        int32 RY = FMath::RandRange(1, 4);
        FVector SpawnLoc = GetActorLocation() + FVector((RX - 3.5f) * TileSize, (RY - 2.5f) * TileSize, 50.f);

        TSubclassOf<AActor> EnemyClass = MyTemplate->PotentialEnemies[FMath::RandRange(0, MyTemplate->PotentialEnemies.Num()-1)].EnemyClass;
        AActor* Enemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLoc, FRotator::ZeroRotator);
        if (Enemy)
        {
            SpawnedEnemies.Add(Enemy);
        }
    }
}

void ARoomBase::SetTileInLayer(int32 Layer, int32 X, int32 Y, const FRoomTileInfo& TileInfo)
{
    if (!TileMapComponent) return;
    FPaperTileInfo PaperTile;
    if (TileInfo.TileSet && TileInfo.PackedTileIndex >= 0) {
        PaperTile.TileSet = TileInfo.TileSet;
        PaperTile.PackedTileIndex = TileInfo.PackedTileIndex;
    } else {
        PaperTile.TileSet = nullptr; PaperTile.PackedTileIndex = -1;
    }
    TileMapComponent->SetTile(X, Y, Layer, PaperTile);
}

void ARoomBase::TrySpawnChest()
{
    if (bHasGeneratedChest || !MyTemplate || !MyTemplate->ChestClass) return;
    if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->ChestSpawnChance)
    {
        bHasGeneratedChest = true;
        GetWorld()->SpawnActor<AActor>(MyTemplate->ChestClass, GetActorLocation(), FRotator::ZeroRotator);
    }
}
