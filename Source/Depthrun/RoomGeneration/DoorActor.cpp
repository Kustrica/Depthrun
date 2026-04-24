// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DoorActor.h"
#include "Components/BoxComponent.h"
#include "PaperFlipbookComponent.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetBoxExtent(FVector(32.f, 32.f, 32.f));
    CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));

    SpriteComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void ADoorActor::OpenDoor()
{
    if (bIsOpen) return;
    bIsOpen = true;

    if (OpenFlipbook)
    {
        SpriteComponent->SetFlipbook(OpenFlipbook);
    }
    else
    {
        SetActorHiddenInGame(true);
    }

    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADoorActor::CloseDoor()
{
    if (!bIsOpen) return;
    bIsOpen = false;

    SetActorHiddenInGame(false);
    if (ClosedFlipbook)
    {
        SpriteComponent->SetFlipbook(ClosedFlipbook);
    }
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
