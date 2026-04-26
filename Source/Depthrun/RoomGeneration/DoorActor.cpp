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
    CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 40.f));
    CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComponent->SetCollisionProfileName(TEXT("BlockAll"));
    SpriteComponent->SetTranslucentSortPriority(20);

    // Start open: hidden + no collision (managed via CollisionBox only).
    bIsOpen = true;
    SpriteComponent->SetHiddenInGame(true);
}

void ADoorActor::InitializeDoor(UPaperSprite* DoorSprite, bool bVerticalDoor, const FRotator& SpriteRotation, float VisualScale,
                                bool bInUseSpriteCollision)
{
    bUseSpriteCollision = bInUseSpriteCollision;

    // No actor rotation — keep world axes aligned.
    // CollisionBox extents are set per-orientation to match doorway.

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
        // THIN across the wall thickness (≈12). The previous values were
        // swapped, which caused the player to pass through one side and be
        // blocked from the other (asymmetric overlap with adjacent walls).
        if (bVerticalDoor)
        {
            // Left/Right passage: span along X, thin along Y
            CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 40.f));
        }
        else
        {
            // Top/Bottom passage: span along Y, thin along X
            CollisionBox->SetBoxExtent(FVector(12.f, 34.f, 40.f));
        }

        if (bUseSpriteCollision)
        {
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void ADoorActor::SetDoorCollisionEnabled(bool bEnabled)
{
    const ECollisionEnabled::Type Mode = bEnabled
                                             ? ECollisionEnabled::QueryAndPhysics
                                             : ECollisionEnabled::NoCollision;

    if (bUseSpriteCollision)
    {
        if (SpriteComponent)
        {
            SpriteComponent->SetCollisionProfileName(TEXT("BlockAll"));
            SpriteComponent->SetCollisionEnabled(Mode);
            SpriteComponent->SetGenerateOverlapEvents(false);
        }

        if (CollisionBox)
        {
            CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        return;
    }

    if (CollisionBox)
    {
        CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
        CollisionBox->SetCollisionEnabled(Mode);
        CollisionBox->SetGenerateOverlapEvents(false);
    }

    if (SpriteComponent)
    {
        SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
