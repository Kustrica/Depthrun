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
    CollisionBox->SetBoxExtent(FVector(20.f, 20.f, 50.f));

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
}

void ADoorActor::InitializeDoor(UPaperSprite* DoorSprite)
{
    if (SpriteComponent && DoorSprite)
    {
        SpriteComponent->SetSprite(DoorSprite);
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
