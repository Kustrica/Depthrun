// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "DoorActor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"
#include "PaperTileLayer.h"
#include "PaperTileMap.h"
#include "PaperTileMapComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "RoomGeneratorSubsystem.h"
#include "RoomTemplate.h"
#include "TimerManager.h"

namespace {
constexpr float BaseTileSize = 16.0f;
const FVector LegacySpawnOffset = FVector(-96.f, 144.f, 0.f);

float ResolveWorldScale(const URoomTemplate* Template) {
  if (!Template || Template->WorldScale <= 0.01f) {
    return 2.6f;
  }
  return Template->WorldScale;
}

FVector ResolveSpawnOffset(const URoomTemplate* Template) {
  if (!Template) {
    return FVector::ZeroVector;
  }

  // Keep old assets (saved with zero offset) aligned with the current room grid.
  if (Template->SpawnOffset.IsNearlyZero(0.01f)) {
    return LegacySpawnOffset;
  }

  return Template->SpawnOffset;
}

float ResolveTileSize(const URoomTemplate* Template) {
  return BaseTileSize * ResolveWorldScale(Template);
}

FVector MakeTileOffset(float TileSize, int32 TileX, int32 TileY, float Z) {
  return FVector((2.5f - static_cast<float>(TileY)) * TileSize,
                 (static_cast<float>(TileX) - 3.5f) * TileSize, Z);
}

FRotator MakeCollisionSafeRotation(const FRotator& SourceRotation) {
  return FRotator(0.f, SourceRotation.Yaw, 0.f);
}

void ApplyVisualScale(AActor* SpawnedActor, float Scale) {
  if (!IsValid(SpawnedActor)) {
    return;
  }

  if (APaperCharacter* PaperCharacter = Cast<APaperCharacter>(SpawnedActor)) {
    if (UPaperFlipbookComponent* Sprite = PaperCharacter->GetSprite()) {
      Sprite->SetRelativeScale3D(FVector(Scale));
    }
    return;
  }

  SpawnedActor->SetActorScale3D(FVector(Scale));
}
} // namespace

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
  TileMapComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 90.f));
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
  const float TileSize = ResolveTileSize(MyTemplate);

  TileMapComponent->SetTileMap(Template->TileMapAsset);
  TileMapComponent->MakeTileMapEditable();
  TileMapComponent->SetRelativeRotation(MyTemplate->TileMapRotation);
  TileMapComponent->SetRelativeLocation(FVector(0.f, 0.f, MyTemplate->TileMapZ));
  TileMapComponent->SetRelativeScale3D(FVector(ResolveWorldScale(MyTemplate)));

  RoomBounds->SetBoxExtent(FVector(3.0f * TileSize, 4.0f * TileSize, 120.f));
  GenerateProps(bHasTop, bHasBottom, bHasLeft, bHasRight);
}

void ARoomBase::GenerateProps(bool bHasTop, bool bHasBottom, bool bHasLeft,
                               bool bHasRight) {
  if (!MyTemplate)
    return;

  OccupiedTiles.Empty();
  SpawnedDoors.Empty();
  const float TileSize = ResolveTileSize(MyTemplate);
  const float VisualScale = ResolveWorldScale(MyTemplate);
  const FVector RoomOrigin = GetActorLocation();
  const FVector SpawnOffset = ResolveSpawnOffset(MyTemplate);
  const FActorSpawnParameters SpawnParams = [] {
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return Params;
  }();

  auto HandleDoorSide = [&](bool bExists, const FVector& SideOffset,
                             UPaperSprite *DoorSprite, const FIntPoint& T1,
                             const FIntPoint& T2, bool bVerticalDoor) {
    if (!bExists || !MyTemplate->DoorClass) {
      return;
    }

    FVector SpawnLoc = RoomOrigin + SpawnOffset + SideOffset;
    SpawnLoc.Z = MyTemplate->DoorZ;
    ADoorActor* Door = GetWorld()->SpawnActor<ADoorActor>(
        MyTemplate->DoorClass, SpawnLoc,
        MakeCollisionSafeRotation(MyTemplate->DoorRotation), SpawnParams);
    if (!Door) {
      return;
    }

    Door->InitializeDoor(DoorSprite, bVerticalDoor, MyTemplate->DoorRotation,
                         VisualScale);
    Door->OpenDoor();
    Door->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    SpawnedDoors.Add(Door);
    OccupiedTiles.Add(T1);
    OccupiedTiles.Add(T2);
  };

  // Top/Bottom use horizontal doorway art, Left/Right use vertical doorway art.
  HandleDoorSide(bHasTop, FVector(TileSize * 3.f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 0),
                 FIntPoint(4, 0), false);
  HandleDoorSide(bHasBottom, FVector(-TileSize * 3.f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 5),
                 FIntPoint(4, 5), false);
  HandleDoorSide(bHasLeft, FVector(0.f, -TileSize * 4.f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(0, 2),
                 FIntPoint(0, 3), true);
  HandleDoorSide(bHasRight, FVector(0.f, TileSize * 4.f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(7, 2),
                 FIntPoint(7, 3), true);

  // 1. Spawning torches (always non-blocking to avoid path dead-ends)
  for (const FIntPoint &Spot : MyTemplate->TorchSpots) {
    if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->TorchSpawnChance) {
      if (MyTemplate->TorchClass) {
        const FVector TorchLoc = RoomOrigin + SpawnOffset +
                                 MakeTileOffset(TileSize, Spot.X, Spot.Y,
                                                MyTemplate->PropsZ);
        AActor *Torch = GetWorld()->SpawnActor<AActor>(
            MyTemplate->TorchClass, TorchLoc, MyTemplate->PropRotation,
            SpawnParams);
        if (Torch) {
          ApplyVisualScale(Torch, VisualScale);
          Torch->AttachToActor(this,
                               FAttachmentTransformRules::KeepWorldTransform);
          Torch->SetActorEnableCollision(false);
          OccupiedTiles.Add(Spot);
        }
      }
    }
  }

  // 2. Decorative elements (always non-blocking by request)
  int32 DecorCount = FMath::RandRange(MyTemplate->MinProps, MyTemplate->MaxProps);
  for (int32 i = 0; i < DecorCount; ++i) {
    int32 RX = FMath::RandRange(1, 6);
    int32 RY = FMath::RandRange(1, 4);

    if (OccupiedTiles.Contains(FIntPoint(RX, RY)))
      continue;

    FVector DecorLoc = RoomOrigin + SpawnOffset +
                       MakeTileOffset(TileSize, RX, RY, MyTemplate->PropsZ);

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
          SelectedClass, DecorLoc, MyTemplate->PropRotation, SpawnParams);
      if (Decor) {
        ApplyVisualScale(Decor, VisualScale);
        Decor->AttachToActor(this,
                             FAttachmentTransformRules::KeepWorldTransform);
        OccupiedTiles.Add(FIntPoint(RX, RY));

        if (SelectedClass == MyTemplate->FloorObstacleClass) {
          Decor->SetActorEnableCollision(false);
          if (UPaperSpriteComponent* SpriteComp = Decor->FindComponentByClass<UPaperSpriteComponent>()) {
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

    if (MyTemplate->TrapdoorClass) {
      FVector HatchLoc = GetActorLocation() + ResolveSpawnOffset(MyTemplate);
      HatchLoc.Z = MyTemplate->PropsZ;
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      AActor* Hatch = GetWorld()->SpawnActor<AActor>(
          MyTemplate->TrapdoorClass, HatchLoc, MyTemplate->PropRotation,
          SpawnParams);
      if (Hatch) {
        ApplyVisualScale(Hatch, ResolveWorldScale(MyTemplate));
      }
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
  const float TileSize = ResolveTileSize(MyTemplate);
  const float VisualScale = ResolveWorldScale(MyTemplate);
  const FVector SpawnOffset = ResolveSpawnOffset(MyTemplate);
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  float TotalWeight = 0.0f;
  for (const FEnemySpawnInfo& Info : MyTemplate->PotentialEnemies) {
      TotalWeight += Info.Weight;
  }

  for (int32 i = 0; i < Count; ++i) {
    if (MyTemplate->PotentialEnemies.Num() == 0 || TotalWeight <= 0.f)
      break;

    int32 RX = FMath::RandRange(1, 6);
    int32 RY = FMath::RandRange(1, 4);
    FVector SpawnLoc = GetActorLocation() + SpawnOffset +
                       MakeTileOffset(TileSize, RX, RY, MyTemplate->EnemyZ);

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
                                                       MakeCollisionSafeRotation(MyTemplate->EnemyRotation), SpawnParams);
        if (Enemy) {
          ApplyVisualScale(Enemy, VisualScale);
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
    
  if (MyTemplate->RoomType == ERoomType::Start)
    return;

  if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->ChestSpawnChance) {
    bHasGeneratedChest = true;
    
    const float TileSize = ResolveTileSize(MyTemplate);
    float OffsetX = FMath::RandRange(-1.0f, 1.0f) * TileSize;
    float OffsetY = FMath::RandRange(-1.0f, 1.0f) * TileSize;
    
    FVector ChestLoc = GetActorLocation() + ResolveSpawnOffset(MyTemplate) + FVector(OffsetX, OffsetY, 0.f);
    ChestLoc.Z = MyTemplate->PropsZ;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Chest = GetWorld()->SpawnActor<AActor>(MyTemplate->ChestClass, ChestLoc,
                                   MyTemplate->PropRotation, SpawnParams);
    if (Chest) {
        ApplyVisualScale(Chest, ResolveWorldScale(MyTemplate));
        Chest->SetActorEnableCollision(false);
    }
  }
}
