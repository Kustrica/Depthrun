// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "ChestActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "PaperSpriteComponent.h"

AChestActor::AChestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetBoxExtent(FVector(14.f, 14.f, 20.f));
    CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetGenerateOverlapEvents(true);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComponent->SetTranslucentSortPriority(20);
}

void AChestActor::BeginPlay()
{
    Super::BeginPlay();
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AChestActor::OnChestOverlap);
}

void AChestActor::OnChestOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult)
{
    if (bOpened || !OtherActor)
        return;

    APawn* OverlapPawn = Cast<APawn>(OtherActor);
    if (!OverlapPawn || !OverlapPawn->IsPlayerControlled())
        return;

    bOpened = true;
    UE_LOG(LogTemp, Log, TEXT("[Chest] You opened the chest!"));
}
