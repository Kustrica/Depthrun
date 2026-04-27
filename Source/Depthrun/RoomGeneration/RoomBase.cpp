// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "DoorActor.h"
#include "Enemy/BaseEnemy.h"
#include "TrapdoorActor.h"
#include "ChestActor.h"
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

bool ARoomBase::IsCombatActive() const
{
	if (!bIsActive || bIsCleared) { return false; }
	for (const TWeakObjectPtr<AActor>& Enemy : SpawnedEnemies)
	{
		if (Enemy.IsValid()) { return true; }
	}
	return false;
}

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
  TileMapComponent->SetCollisionProfileName(TEXT("BlockAll"));
  TileMapComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  TileMapComponent->SetGenerateOverlapEvents(false);
}

void ARoomBase::BeginPlay() {
  Super::BeginPlay();

  if (bPendingSetup && MyTemplate && MyTemplate->bUseProceduralWallColliders)
  {
    const float TileSize = ResolveTileSize(MyTemplate);
    BuildWallColliders(TileSize, bPendingHasTop, bPendingHasBottom,
                       bPendingHasLeft, bPendingHasRight);
  }
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
  TileMapComponent->SetRelativeScale3D(FVector(ResolveWorldScale(MyTemplate)));
  TileMapComponent->SetTranslucentSortPriority(0);

  // Center the TileMap around the actor origin.
  // With rotation (-90, 0, 90):
  //   tile rows (6) grow in World -X direction -> shift +HalfX to center
  //   tile cols (8) grow in World +Y direction -> shift -HalfY to center
  const float HalfX = 3.0f * TileSize; // half of 6 rows
  const float HalfY = 4.0f * TileSize; // half of 8 cols
  // TileMapVisualOffset lets you nudge ONLY the art without moving colliders/props.
  const FVector2D VO = MyTemplate->TileMapVisualOffset;
  TileMapComponent->SetRelativeLocation(FVector(HalfX + VO.X, -HalfY + VO.Y, MyTemplate->TileMapZ));

  // Trigger only over the inner floor area (1.5 tiles inset) so it does not
  // fire when the player is just stepping into the doorway.
  const float TriggerInset = TileSize * 1.5f;
  RoomBounds->SetBoxExtent(FVector(FMath::Max(HalfX - TriggerInset, TileSize), FMath::Max(HalfY - TriggerInset, TileSize), 120.f));

  const int32 FloorLayer = 0;

  // Top door: clear tiles, then place A at (3,0) and B at (4,0)
  if (bHasTop) {
    const FRoomTileInfo Empty;
    for (int32 L = 0; L < 3; ++L) { SetTileInLayer(L, 3, 0, Empty); SetTileInLayer(L, 4, 0, Empty); }
    SetTileInLayer(FloorLayer, 3, 0, MyTemplate->DoorShadowTop_A);
    SetTileInLayer(FloorLayer, 4, 0, MyTemplate->DoorShadowTop_B);
  }
  // Bottom door
  if (bHasBottom) {
    const FRoomTileInfo Empty;
    for (int32 L = 0; L < 3; ++L) { SetTileInLayer(L, 3, 5, Empty); SetTileInLayer(L, 4, 5, Empty); }
    SetTileInLayer(FloorLayer, 3, 5, MyTemplate->DoorShadowBottom_A);
    SetTileInLayer(FloorLayer, 4, 5, MyTemplate->DoorShadowBottom_B);
  }
  // Left door
  if (bHasLeft) {
    const FRoomTileInfo Empty;
    for (int32 L = 0; L < 3; ++L) { SetTileInLayer(L, 0, 2, Empty); SetTileInLayer(L, 0, 3, Empty); }
    SetTileInLayer(FloorLayer, 0, 2, MyTemplate->DoorShadowLeft_A);
    SetTileInLayer(FloorLayer, 0, 3, MyTemplate->DoorShadowLeft_B);
  }
  // Right door
  if (bHasRight) {
    const FRoomTileInfo Empty;
    for (int32 L = 0; L < 3; ++L) { SetTileInLayer(L, 7, 2, Empty); SetTileInLayer(L, 7, 3, Empty); }
    SetTileInLayer(FloorLayer, 7, 2, MyTemplate->DoorShadowRight_A);
    SetTileInLayer(FloorLayer, 7, 3, MyTemplate->DoorShadowRight_B);
  }

  // Rebuild tilemap collision (works if TileSet has tile-level collision shapes).
  if (TileMapComponent)
  {
    TileMapComponent->SetCollisionProfileName(TEXT("BlockAll"));
    TileMapComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMapComponent->SetGenerateOverlapEvents(false);
    TileMapComponent->RebuildCollision();
  }

  // Save door flags — BuildWallColliders is deferred to BeginPlay so that
  // Unreal's physics state is fully initialized before we register components.
  bPendingHasTop    = bHasTop;
  bPendingHasBottom = bHasBottom;
  bPendingHasLeft   = bHasLeft;
  bPendingHasRight  = bHasRight;
  bPendingSetup     = true;

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
  const FActorSpawnParameters SpawnParams = [] {
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return Params;
  }();

  // Do not spawn door actors in the Start room — the passages must always
  // be open and unblocked. Doors would block the player even when "open"
  // due to their BoxComponent Z-extent overlapping the player collision.
  const bool bIsStartRoom = (MyTemplate->RoomType == ERoomType::Start);

  auto HandleDoorSide = [&](bool bExists, const FVector& SideOffset,
                             UPaperSprite *DoorSprite, const FIntPoint& T1,
                             const FIntPoint& T2, bool bVerticalDoor) {
    if (!bExists || !MyTemplate->DoorClass || bIsStartRoom) {
      return;
    }

    FVector SpawnLoc = RoomOrigin + SideOffset;
    SpawnLoc.Z = MyTemplate->DoorLockedZ;
    // Spawn with ZeroRotator so CollisionBox extents map directly to world axes.
    // SpriteComponent receives PropRotation inside InitializeDoor.
    ADoorActor* Door = GetWorld()->SpawnActor<ADoorActor>(
        MyTemplate->DoorClass, SpawnLoc,
        FRotator::ZeroRotator, SpawnParams);
    if (!Door) {
      return;
    }

    Door->InitializeDoor(DoorSprite, bVerticalDoor, MyTemplate->PropRotation,
                         VisualScale);
    // Door starts open (hidden, no collision) from the constructor.
    // ActivateRoom() will call CloseDoor() to block the player.
    Door->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    SpawnedDoors.Add(Door);
    OccupiedTiles.Add(T1);
    OccupiedTiles.Add(T2);
  };

  // Top/Bottom use horizontal doorway art, Left/Right use vertical doorway art.
  HandleDoorSide(bHasTop, FVector(TileSize * 2.5f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 0),
                 FIntPoint(4, 0), false);
  HandleDoorSide(bHasBottom, FVector(-TileSize * 2.5f, 0.f, 0.f),
                 MyTemplate->HorizontalDoorSprite, FIntPoint(3, 5),
                 FIntPoint(4, 5), false);
  HandleDoorSide(bHasLeft, FVector(0.f, -TileSize * 3.5f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(0, 2),
                 FIntPoint(0, 3), true);
  HandleDoorSide(bHasRight, FVector(0.f, TileSize * 3.5f, 0.f),
                 MyTemplate->VerticalDoorSprite, FIntPoint(7, 2),
                 FIntPoint(7, 3), true);

  // 1. Spawning torches (always non-blocking to avoid path dead-ends)
  for (const FIntPoint &Spot : MyTemplate->TorchSpots) {
    if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->TorchSpawnChance) {
      if (MyTemplate->TorchClass) {
        const FVector TorchLoc = RoomOrigin +
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

  // 2. Decorative elements
  int32 DecorCount = FMath::RandRange(MyTemplate->MinProps, MyTemplate->MaxProps);
  for (int32 i = 0; i < DecorCount; ++i) {
    // Keep at least 2 tiles away from walls so the player can pass between
    // an obstacle and the wall. Cols 2..5, rows 2..3 (centre of the room).
    int32 RX = FMath::RandRange(2, 5);
    int32 RY = FMath::RandRange(2, 3);

    if (OccupiedTiles.Contains(FIntPoint(RX, RY)))
      continue;

    FVector DecorLoc = RoomOrigin +
                       MakeTileOffset(TileSize, RX, RY, MyTemplate->PropsZ);

    TSubclassOf<AActor> SelectedClass = nullptr;
    float Rand = FMath::FRand();
    if (Rand < 0.33f)
      SelectedClass = MyTemplate->SkullDecorClass;
    else if (Rand < 0.66f)
      SelectedClass = MyTemplate->BonesDecorClass;
    else if (MyTemplate->FloorObstacleClass)
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
          // Pick a random sprite variant. Collision mode is template-driven:
          // sprite geometry collision (preferred) or no collision.
          if (UPaperSpriteComponent* SpriteComp = Decor->FindComponentByClass<UPaperSpriteComponent>()) {
              if (MyTemplate->ObstacleVariants.Num() > 0) {
                  int32 VariantIdx = FMath::RandRange(0, MyTemplate->ObstacleVariants.Num() - 1);
                  SpriteComp->SetSprite(MyTemplate->ObstacleVariants[VariantIdx]);
              }
              if (MyTemplate->bUseObstacleSpriteCollision) {
                  SpriteComp->SetCollisionProfileName(TEXT("BlockAll"));
                  SpriteComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                  SpriteComp->SetGenerateOverlapEvents(false);
                  Decor->SetActorEnableCollision(true);
              } else {
                  SpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                  Decor->SetActorEnableCollision(false);
              }
              SpriteComp->SetTranslucentSortPriority(5);
          }
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
  if (bIsCleared || bIsActive)
    return;
  bIsActive = true;

  if (!bHasSpawnedEnemies) {
    SpawnEnemies();
    bHasSpawnedEnemies = true;
  }

  if (SpawnedEnemies.Num() == 0) {
    DeactivateRoom();
    return;
  }

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
  bIsActive = false;
  GetWorldTimerManager().ClearTimer(TimerHandle_CheckEnemies);

  for (AActor *Door : SpawnedDoors) {
    if (ADoorActor *DA = Cast<ADoorActor>(Door))
      DA->OpenDoor();
  }

  if (MyTemplate && MyTemplate->RoomType == ERoomType::Boss) {
    UE_LOG(LogTemp, Warning,
           TEXT("[DungeonGen] BOSS CLEARED! EXIT HATCH OPEN."));

    if (!MyTemplate->TrapdoorClass) {
      UE_LOG(LogTemp, Error, TEXT("[DungeonGen] TrapdoorClass is NULL - assign BP_Trapdoor in Boss Room DataAsset!"));
    } else {
      const float TileSize = ResolveTileSize(MyTemplate);
      // Try center tiles: avoid OccupiedTiles
      TArray<FIntPoint> CenterTiles = { {3,2}, {4,2}, {3,3}, {4,3}, {2,2}, {5,2} };
      FIntPoint SelectedTile = FIntPoint(3, 2);
      for (const FIntPoint& Candidate : CenterTiles) {
        if (!OccupiedTiles.Contains(Candidate)) {
          SelectedTile = Candidate;
          break;
        }
      }
      FVector HatchLoc = GetActorLocation() + MakeTileOffset(TileSize, SelectedTile.X, SelectedTile.Y, 0.f);
      HatchLoc.Z = MyTemplate->TrapdoorZ;
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      ATrapdoorActor* Hatch = GetWorld()->SpawnActor<ATrapdoorActor>(
          MyTemplate->TrapdoorClass, HatchLoc, MyTemplate->PropRotation, SpawnParams);
      if (!Hatch) {
        UE_LOG(LogTemp, Error, TEXT("[DungeonGen] SpawnActor FAILED for TrapdoorClass at %s"), *HatchLoc.ToString());
      } else {
        ApplyVisualScale(Hatch, ResolveWorldScale(MyTemplate));
        OccupiedTiles.Add(SelectedTile); // prevent chest from spawning on top of hatch
        UE_LOG(LogTemp, Warning, TEXT("[DungeonGen] EXIT HATCH SPAWNED: class=%s tile=(%d,%d) loc=%s"),
               *MyTemplate->TrapdoorClass->GetName(), SelectedTile.X, SelectedTile.Y, *HatchLoc.ToString());
      }
    }
  }

  TrySpawnChest();
}

void ARoomBase::SpawnEnemies() {
  if (!MyTemplate || MyTemplate->RoomType == ERoomType::Start)
    return;

  SpawnedEnemies.Reset();

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

    // Spawn enemies in the inner area only — keep distance from doors
    // so the player doesn't get stuck in an enemy when entering the room.
    int32 RX = FMath::RandRange(2, 5);
    int32 RY = FMath::RandRange(2, 3);
    if (OccupiedTiles.Contains(FIntPoint(RX, RY)))
      continue;

    // Use EnemyLockedZ directly from DataAsset (no clamping applied here).
    FVector SpawnLoc = GetActorLocation() +
                       MakeTileOffset(TileSize, RX, RY, MyTemplate->EnemyLockedZ);

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
          // Lock the enemy to the correct Z plane from DataAsset.
          // This MUST be called after SpawnActor because bSnapToPlaneAtStart=false,
          // and the CharacterMovement plane origin defaults to Z=0.
          if (ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Enemy))
          {
            BaseEnemy->SetLockedZ(MyTemplate->EnemyLockedZ);
          }
          // Enemies stay at scale 1.0 — do not apply the tilemap visual scale.
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
    PaperTile = FPaperTileInfo();
  }
  TileMapComponent->SetTile(X, Y, Layer, PaperTile);
}

void ARoomBase::BuildWallColliders(float TileSize, bool bHasTop, bool bHasBottom,
                                   bool bHasLeft, bool bHasRight)
{
  // Destroy any previously created wall colliders (room re-setup guard).
  for (UBoxComponent* Old : WallColliders)
  {
    if (Old) Old->DestroyComponent();
  }
  WallColliders.Empty();

  // Room layout (actor origin = room center, rotation (-90,0,90)):
  //   World X: rows grow in -X, so Top wall is at +HalfX, Bottom at -HalfX
  //   World Y: cols grow in +Y, so Left wall is at -HalfY, Right at +HalfY
  // Tile size: 6 rows x 8 cols.
  const float HalfX   = 3.0f * TileSize;
  const float HalfY   = 4.0f * TileSize;
  const float WallT   = TileSize * 0.5f;   // half-extent of wall thickness
  const float DoorHW  = TileSize;          // half-width of doorway (2 tiles / 2)

  // Helper: creates one BoxComponent attached to this actor.
  auto AddWall = [&](FVector RelCenter, FVector HalfExtent)
  {
    UBoxComponent* Box = NewObject<UBoxComponent>(this);
    Box->SetupAttachment(RootComponent);
    Box->RegisterComponent();
    Box->SetRelativeLocation(RelCenter);
    Box->SetBoxExtent(HalfExtent);
    Box->SetCollisionProfileName(TEXT("BlockAll"));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WallColliders.Add(Box);
  };

  // ── Top wall (WorldX = +HalfX - WallT) ─────────────────────────────────
  // Full span Y: [-HalfY .. +HalfY]. Doorway center at Y=0, half-width=DoorHW.
  if (bHasTop)
  {
    // Left segment: Y in [-HalfY .. -DoorHW]
    float SegLen = (HalfY - DoorHW) * 0.5f;
    AddWall(FVector(HalfX - WallT, -DoorHW - SegLen, 0.f), FVector(WallT, SegLen, 40.f));
    // Right segment: Y in [+DoorHW .. +HalfY]
    AddWall(FVector(HalfX - WallT,  DoorHW + SegLen, 0.f), FVector(WallT, SegLen, 40.f));
  }
  else
  {
    AddWall(FVector(HalfX - WallT, 0.f, 0.f), FVector(WallT, HalfY, 40.f));
  }

  // ── Bottom wall (WorldX = -HalfX + WallT) ───────────────────────────────
  if (bHasBottom)
  {
    float SegLen = (HalfY - DoorHW) * 0.5f;
    AddWall(FVector(-HalfX + WallT, -DoorHW - SegLen, 0.f), FVector(WallT, SegLen, 40.f));
    AddWall(FVector(-HalfX + WallT,  DoorHW + SegLen, 0.f), FVector(WallT, SegLen, 40.f));
  }
  else
  {
    AddWall(FVector(-HalfX + WallT, 0.f, 0.f), FVector(WallT, HalfY, 40.f));
  }

  // ── Left wall (WorldY = -HalfY + WallT) ─────────────────────────────────
  // Interior span X: [-(HalfX-WallT) .. +(HalfX-WallT)]. Doorway at X=0.
  const float InnerHalfX = HalfX - WallT;
  if (bHasLeft)
  {
    float SegLen = (InnerHalfX - DoorHW) * 0.5f;
    AddWall(FVector(-DoorHW - SegLen, -HalfY + WallT, 0.f), FVector(SegLen, WallT, 40.f));
    AddWall(FVector( DoorHW + SegLen, -HalfY + WallT, 0.f), FVector(SegLen, WallT, 40.f));
  }
  else
  {
    AddWall(FVector(0.f, -HalfY + WallT, 0.f), FVector(InnerHalfX, WallT, 40.f));
  }

  // ── Right wall (WorldY = +HalfY - WallT) ────────────────────────────────
  if (bHasRight)
  {
    float SegLen = (InnerHalfX - DoorHW) * 0.5f;
    AddWall(FVector(-DoorHW - SegLen, HalfY - WallT, 0.f), FVector(SegLen, WallT, 40.f));
    AddWall(FVector( DoorHW + SegLen, HalfY - WallT, 0.f), FVector(SegLen, WallT, 40.f));
  }
  else
  {
    AddWall(FVector(0.f, HalfY - WallT, 0.f), FVector(InnerHalfX, WallT, 40.f));
  }
}

void ARoomBase::TrySpawnChest() {
  if (bHasGeneratedChest || !MyTemplate || !MyTemplate->ChestClass)
    return;
    
  if (MyTemplate->RoomType == ERoomType::Start)
    return;

  if (FMath::FRandRange(0.f, 100.f) <= MyTemplate->ChestSpawnChance) {
    const float TileSize = ResolveTileSize(MyTemplate);
    
    // Try to find a tile position not occupied by torches/obstacles
    // Valid tiles: center area (2..5, 2..3) avoiding OccupiedTiles
    TArray<FIntPoint> ValidTiles;
    for (int32 X = 2; X <= 5; ++X) {
      for (int32 Y = 2; Y <= 3; ++Y) {
        FIntPoint Tile(X, Y);
        if (!OccupiedTiles.Contains(Tile)) {
          ValidTiles.Add(Tile);
        }
      }
    }
    
    // If no valid tiles, skip chest spawn this time
    if (ValidTiles.Num() == 0) {
      UE_LOG(LogTemp, Warning, TEXT("[RoomGen] No valid tile for chest spawn - all center tiles occupied."));
      return;
    }
    
    bHasGeneratedChest = true;
    
    // Pick random valid tile
    FIntPoint SelectedTile = ValidTiles[FMath::RandRange(0, ValidTiles.Num() - 1)];
    FVector ChestLoc = GetActorLocation() + 
                       MakeTileOffset(TileSize, SelectedTile.X, SelectedTile.Y, 0.f);
    ChestLoc.Z = MyTemplate->PropsZ;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AChestActor* Chest = GetWorld()->SpawnActor<AChestActor>(MyTemplate->ChestClass, ChestLoc,
                                       MyTemplate->PropRotation, SpawnParams);
    if (Chest) {
        ApplyVisualScale(Chest, ResolveWorldScale(MyTemplate));
        UE_LOG(LogTemp, Log, TEXT("[RoomGen] Chest spawned at tile (%d,%d)."), SelectedTile.X, SelectedTile.Y);
    }
  }
}
