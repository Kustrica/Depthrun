// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "DoorActor.h"
#include "Enemy/BaseEnemy.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "PaperTileLayer.h"
#include "PaperTileMap.h"
#include "PaperTileMapComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "RoomGeneratorSubsystem.h"
#include "RoomTemplate.h"
#include "TimerManager.h"

ARoomBase::ARoomBase() {
  PrimaryActorTick.bCanEverTick = false;

  RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
  RootComponent = RoomBounds;
  RoomBounds->SetBoxExtent(FVector(150.f, 120.f, 100.f));
  RoomBounds->SetCollisionProfileName(TEXT("Trigger"));
  RoomBounds->OnComponentBeginOverlap.AddDynamic(this, &ARoomBase::OnRoomEntry);

  TileMapComponent =
      CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("TileMapComponent"));
  TileMapComponent->SetupAttachment(RootComponent);
  // Initial rotation, will be overwritten by Template in SetupRoom
  TileMapComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 90.f));
  // Scale uniformly to prevent squashed rooms
  TileMapComponent->SetRelativeScale3D(FVector(2.6f, 2.6f, 2.6f));
}

void ARoomBase::BeginPlay() { 
  Super::BeginPlay(); 
}

void ARoomBase::SetupRoom(URoomTemplate *Template, bool bHasTop, bool bHasBottom,
                        bool bHasLeft, bool bHasRight) {
  if (!Template || !TileMapComponent)
    return;
  MyTemplate = Template;

  TileMapComponent->SetTileMap(Template->TileMapAsset);
  TileMapComponent->MakeTileMapEditable();
  TileMapComponent->SetRelativeRotation(MyTemplate->TileMapRotation);
  TileMapComponent->SetRelativeLocation(FVector(0.f, 0.f, MyTemplate->TileMapZ));

  // Door shadow logic moved to GenerateProps or handled here if needed
  // For now, let's just make sure TileMap uses DefaultRoomRotation
  GenerateProps(bHasTop, bHasBottom, bHasLeft, bHasRight);
}

void ARoomBase::GenerateProps(bool bHasTop, bool bHasBottom, bool bHasLeft,
                               bool bHasRight) {
  if (!MyTemplate)
    return;

  OccupiedTiles.Empty();
  const float TileSize = 16.0f * 2.6f;
  const FVector RoomOrigin = GetActorLocation();

  auto HandleDoorSide = [&](bool bExists, FVector SpawnOffset,
                             UPaperSprite *DoorSprite, FIntPoint T1, FIntPoint T2) {
    if (bExists) {
      if (MyTemplate->DoorClass) {
        FVector SpawnLoc = RoomOrigin + SpawnOffset;
        SpawnLoc.Z = MyTemplate->DoorZ;
        AActor *Door = GetWorld()->SpawnActor<AActor>(
            MyTemplate->DoorClass, SpawnLoc, MyTemplate->PropRotation);
        if (Door) {
          Door->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
          Door->AttachToActor(this,
                               FAttachmentTransformRules::KeepWorldTransform);
          if (ADoorActor *DA = Cast<ADoorActor>(Door)) {
            DA->InitializeDoor(DoorSprite);
          }
          SpawnedDoors.Add(Door);
          OccupiedTiles.Add(T1);
          OccupiedTiles.Add(T2);
        }
      }
    }
  };

  HandleDoorSide(bHasTop, FVector(TileSize * 3.f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 0), FIntPoint(4, 0));
  HandleDoorSide(bHasBottom, FVector(-TileSize * 3.f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 5), FIntPoint(4, 5));
  HandleDoorSide(bHasLeft, FVector(0.f, -TileSize * 4.f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(0, 2), FIntPoint(0, 3));
  HandleDoorSide(bHasRight, FVector(0.f, TileSize * 4.f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(7, 2), FIntPoint(7, 3));

  // 1. Spawning torches
  for (const FIntPoint &Spot : MyTemplate->TorchSpots) {
    if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->TorchSpawnChance) {
      if (MyTemplate->TorchClass) {
        FVector TorchLoc = RoomOrigin + FVector((2.5f - Spot.Y) * TileSize,
                                                (Spot.X - 3.5f) * TileSize, MyTemplate->PropsZ);
        AActor *Torch = GetWorld()->SpawnActor<AActor>(
            MyTemplate->TorchClass, TorchLoc, MyTemplate->PropRotation);
        if (Torch) {
          Torch->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
          Torch->AttachToActor(this,
                               FAttachmentTransformRules::KeepWorldTransform);
          
          // Commercial Fix: Torches should NOT have collision
          Torch->SetActorEnableCollision(false);
          
          OccupiedTiles.Add(Spot);
        }
      }
    }
  }

  // 2. Decorative elements (Bones, Skulls, etc)
  int32 DecorCount = FMath::RandRange(MyTemplate->MinProps, MyTemplate->MaxProps);
  for (int32 i = 0; i < DecorCount; ++i) {
    int32 RX = FMath::RandRange(1, 6);
    int32 RY = FMath::RandRange(1, 4);

    if (OccupiedTiles.Contains(FIntPoint(RX, RY)))
      continue;

    FVector DecorLoc = RoomOrigin + FVector((2.5f - RY) * TileSize,
                                            (RX - 3.5f) * TileSize, MyTemplate->PropsZ);

    TSubclassOf<AActor> SelectedClass = nullptr;
    float Rand = FMath::FRand();
    if (Rand < 0.33f)
      SelectedClass = MyTemplate->SkullDecorClass;
    else if (Rand < 0.66f)
      SelectedClass = MyTemplate->BonesDecorClass;
    else
      SelectedClass = MyTemplate->FloorObstacleClass;

    if (SelectedClass) {
      AActor *Decor = GetWorld()->SpawnActor<AActor>(
          SelectedClass, DecorLoc, MyTemplate->PropRotation);
      if (Decor) {
        Decor->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
        Decor->AttachToActor(this,
                             FAttachmentTransformRules::KeepWorldTransform);
        OccupiedTiles.Add(FIntPoint(RX, RY));

        // Commercial Fix: Only Obstacles (торшеры) have collision. Bones/Skulls do not.
        if (SelectedClass == MyTemplate->FloorObstacleClass) {
          Decor->SetActorEnableCollision(true);
          
          if (UPaperSpriteComponent* SpriteComp = Decor->FindComponentByClass<UPaperSpriteComponent>()) {
              SpriteComp->SetCollisionProfileName(TEXT("BlockAll"));
              
              if (MyTemplate->ObstacleVariants.Num() > 0) {
                  int32 VariantIdx = FMath::RandRange(0, MyTemplate->ObstacleVariants.Num() - 1);
                  SpriteComp->SetSprite(MyTemplate->ObstacleVariants[VariantIdx]);
              }
          }
        } else {
          Decor->SetActorEnableCollision(false);
        }
      }
    }
  }
}

void ARoomBase::OnRoomEntry(UPrimitiveComponent *OverlappedComp,
                            AActor *OtherActor, UPrimitiveComponent *OtherComp,
                            int32 OtherBodyIndex, bool bFromSweep,
                            const FHitResult &SweepResult) {
  APawn *Pawn = Cast<APawn>(OtherActor);
  if (bIsCleared || !Pawn || !Pawn->IsPlayerControlled())
    return;

  if (URoomGeneratorSubsystem *Sub =
          GetWorld()->GetSubsystem<URoomGeneratorSubsystem>()) {
    Sub->OnPlayerEnteredRoom(this);
  }
}

void ARoomBase::ActivateRoom() {
  if (bIsCleared)
    return;

  SpawnEnemies();

  // Close all doors
  for (AActor *Door : SpawnedDoors) {
    if (ADoorActor *DA = Cast<ADoorActor>(Door))
      DA->CloseDoor();
  }

  // Start checking for enemy status every 1 second
  GetWorldTimerManager().SetTimer(TimerHandle_CheckEnemies, this,
                                  &ARoomBase::CheckEnemiesStatus, 1.0f, true);
}

void ARoomBase::CheckEnemiesStatus() {
  if (bIsCleared)
    return;

  // Clean up null/dead pointers
  SpawnedEnemies.RemoveAll([](const TWeakObjectPtr<AActor> &Enemy) {
    return !Enemy.IsValid();
  });

  if (SpawnedEnemies.Num() == 0) {
    GetWorldTimerManager().ClearTimer(TimerHandle_CheckEnemies);
    DeactivateRoom();
  }
}

void ARoomBase::DeactivateRoom() {
  if (bIsCleared)
    return;
  bIsCleared = true;

  for (AActor *Door : SpawnedDoors) {
    if (ADoorActor *DA = Cast<ADoorActor>(Door))
      DA->OpenDoor();
  }

  if (MyTemplate && MyTemplate->RoomType == ERoomType::Boss) {
    UE_LOG(LogTemp, Warning,
           TEXT("[DungeonGen] BOSS CLEARED! EXIT HATCH OPEN."));

    // Спавним люк (Hatch) в центре комнаты босса
    if (MyTemplate->TrapdoorClass) {
      FVector HatchLoc = GetActorLocation();
      HatchLoc.Z = MyTemplate->PropsZ;
      AActor* Hatch = GetWorld()->SpawnActor<AActor>(MyTemplate->TrapdoorClass, HatchLoc,
                                     MyTemplate->PropRotation);
      if (Hatch) Hatch->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
      UE_LOG(LogTemp, Warning, TEXT("[DungeonGen] EXIT HATCH SPAWNED."));
    }
  }

  TrySpawnChest();
}

void ARoomBase::SpawnEnemies() {
  if (!MyTemplate || MyTemplate->RoomType == ERoomType::Start)
    return;

  int32 Count =
      FMath::RandRange(MyTemplate->MinEnemies, MyTemplate->MaxEnemies);
  const float TileSize = 16.0f * 2.6f;

  float TotalWeight = 0.0f;
  for (const FEnemySpawnInfo& Info : MyTemplate->PotentialEnemies) {
      TotalWeight += Info.Weight;
  }

  for (int32 i = 0; i < Count; ++i) {
    if (MyTemplate->PotentialEnemies.Num() == 0 || TotalWeight <= 0.f)
      break;

    int32 RX = FMath::RandRange(1, 6);
    int32 RY = FMath::RandRange(1, 4);
    FVector SpawnLoc =
        GetActorLocation() +
        FVector((2.5f - RY) * TileSize, (RX - 3.5f) * TileSize, MyTemplate->EnemyZ);

    TSubclassOf<AActor> EnemyClass = nullptr;
    float Rand = FMath::FRand() * TotalWeight;
    float CurrentSum = 0.0f;
    
    for (const FEnemySpawnInfo& Info : MyTemplate->PotentialEnemies) {
        CurrentSum += Info.Weight;
        if (Rand <= CurrentSum) {
            EnemyClass = Info.EnemyClass;
            break;
        }
    }

    if (EnemyClass) {
        AActor *Enemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLoc,
                                                       MyTemplate->EnemyRotation);
        if (Enemy) {
          Enemy->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
          SpawnedEnemies.Add(Enemy);
        }
    }
  }
}

void ARoomBase::SetTileInLayer(int32 Layer, int32 X, int32 Y,
                               const FRoomTileInfo &TileInfo) {
  if (!TileMapComponent)
    return;
  FPaperTileInfo PaperTile;
  if (TileInfo.TileSet && TileInfo.PackedTileIndex >= 0) {
    PaperTile.TileSet = TileInfo.TileSet;
    PaperTile.PackedTileIndex = TileInfo.PackedTileIndex;
  } else {
    PaperTile.TileSet = nullptr;
    PaperTile.PackedTileIndex = -1;
  }
  TileMapComponent->SetTile(X, Y, Layer, PaperTile);
}

void ARoomBase::TrySpawnChest() {
  if (bHasGeneratedChest || !MyTemplate || !MyTemplate->ChestClass)
    return;
  if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->ChestSpawnChance) {
    bHasGeneratedChest = true;
    FVector ChestLoc = GetActorLocation();
    ChestLoc.Z = MyTemplate->PropsZ;
    AActor* Chest = GetWorld()->SpawnActor<AActor>(MyTemplate->ChestClass, ChestLoc,
                                   MyTemplate->PropRotation);
    if (Chest) Chest->SetActorScale3D(FVector(2.6f, 2.6f, 2.6f));
  }
}
