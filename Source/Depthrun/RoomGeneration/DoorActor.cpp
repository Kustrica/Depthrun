// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DoorActor.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 40.f));

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Doors should be passable until a room is activated.
    bIsOpen = true;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void ADoorActor::InitializeDoor(UPaperSprite* DoorSprite, bool bVerticalDoor, const FRotator& SpriteRotation, float VisualScale)
{
    if (SpriteComponent && DoorSprite)
    {
        SpriteComponent->SetSprite(DoorSprite);
        SpriteComponent->SetRelativeRotation(SpriteRotation);
        SpriteComponent->SetRelativeScale3D(FVector(VisualScale));
    }

    if (CollisionBox)
    {
        // Keep blocker thin along wall depth and wide along doorway.
        if (bVerticalDoor)
        {
            CollisionBox->SetBoxExtent(FVector(12.f, 34.f, 40.f));
        }
        else
        {
            CollisionBox->SetBoxExtent(FVector(34.f, 12.f, 40.f));
        }
    }
}

void ADoorActor::OpenDoor()
{
    if (bIsOpen) return;
    bIsOpen = true;

    // Дверь "исчезает" и перестает блокировать путь
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void ADoorActor::CloseDoor()
{
    if (!bIsOpen) return;
    bIsOpen = false;

    // Дверь появляется и блокирует путь
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
}
