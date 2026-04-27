// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DoorActor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // CollisionBox as root — simplest, most reliable setup.
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    // Z-extent 20 ensures projectiles at Z=15 are blocked (door Z ~3 + 20 covers up to 23).
    CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 20.f));
    CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComponent->SetCollisionProfileName(TEXT("NoCollision"));
    SpriteComponent->SetTranslucentSortPriority(20);

    // Start open: hidden + no collision (managed via CollisionBox only).
    bIsOpen = true;
    SpriteComponent->SetHiddenInGame(true);
}

void ADoorActor::InitializeDoor(UPaperSprite* DoorSprite, bool bVerticalDoor, const FRotator& SpriteRotation, float VisualScale)
{
    // No actor rotation — keep world axes aligned.
    // CollisionBox extents are set per-orientation to match doorway.
    // Sprite is visual-only; collision is ALWAYS handled by thin CollisionBox (Z-extent=8).

    if (SpriteComponent && DoorSprite)
    {
        SpriteComponent->SetSprite(DoorSprite);
        SpriteComponent->SetRelativeRotation(SpriteRotation);
        SpriteComponent->SetRelativeScale3D(FVector(VisualScale));
    }

    if (CollisionBox)
    {
        // Doorway gap on the room boundary:
        //   Top/Bottom doors  → 2-tile gap along world Y axis
        //   Left/Right doors  → 2-tile gap along world X axis
        // The box must SPAN the gap (≈34 = 2 tiles half-extent) and be
        // THIN across the wall thickness (≈12) and THIN in Z (8) for 2D gameplay.
        // Z-extent 20 covers projectile height (enemy Z=15), XY stay thin for doorway fit.
        if (bVerticalDoor)
        {
            // Left/Right passage: span along X, thin along Y
            CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 20.f));
        }
        else
        {
            // Top/Bottom passage: span along Y, thin along X
            CollisionBox->SetBoxExtent(FVector(12.f, 34.f, 20.f));
        }
    }
}

void ADoorActor::SetDoorCollisionEnabled(bool bEnabled)
{
    // Only CollisionBox handles collision; sprite is visual-only.
    // Z-extent 20 covers enemy projectile height while staying tight to gameplay plane.
    const ECollisionEnabled::Type Mode = bEnabled
                                             ? ECollisionEnabled::QueryAndPhysics
                                             : ECollisionEnabled::NoCollision;

    if (CollisionBox)
    {
        CollisionBox->SetCollisionEnabled(Mode);
        if (bEnabled)
        {
            CollisionBox->SetCollisionObjectType(ECC_WorldStatic);
            CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
            CollisionBox->SetGenerateOverlapEvents(false);
        }
    }
}

void ADoorActor::OpenDoor()
{
    if (bIsOpen) return;
    bIsOpen = true;

    if (SpriteComponent) SpriteComponent->SetHiddenInGame(true);
    SetDoorCollisionEnabled(false);
}

void ADoorActor::CloseDoor()
{
    if (!bIsOpen) return;
    bIsOpen = false;

    if (SpriteComponent) SpriteComponent->SetHiddenInGame(false);
    SetDoorCollisionEnabled(true);
}
