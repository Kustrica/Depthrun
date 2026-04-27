// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "TrapdoorActor.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

ATrapdoorActor::ATrapdoorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Root collision box - overlap only for detection
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
    CollisionBox->SetBoxExtent(FVector(34.f, 34.f, 20.f));
    CollisionBox->SetCollisionProfileName(TEXT("OverlapAll"));
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->SetGenerateOverlapEvents(true);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpriteComponent->SetTranslucentSortPriority(30);
}

void ATrapdoorActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event for player detection
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATrapdoorActor::OnTrapdoorOverlap);
}

void ATrapdoorActor::OnTrapdoorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                       bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
        return;

    // Check if overlapping actor is player (Pawn with PlayerController)
    APawn* OverlapPawn = Cast<APawn>(OtherActor);
    if (!OverlapPawn)
    {
        // Try to get pawn from the actor
        AActor* TestActor = OtherActor->GetInstigator();
        OverlapPawn = Cast<APawn>(TestActor);
    }
    
    // Check for Player tag or if it's a player-controlled pawn
    bool bIsPlayer = false;
    if (OverlapPawn && OverlapPawn->IsPlayerControlled())
    {
        bIsPlayer = true;
    }
    else if (OtherActor->ActorHasTag(TEXT("Player")))
    {
        bIsPlayer = true;
    }

    if (!bIsPlayer)
        return;

    // Green log message for player stepping on trapdoor
    UE_LOG(LogTemp, Log, TEXT("\n========================================"));
    UE_LOG(LogTemp, Log, TEXT("[TRAPDOOR] Player stepped on exit hatch!"));
    UE_LOG(LogTemp, Log, TEXT("[TRAPDOOR] Level complete - ready for transition!"));
    UE_LOG(LogTemp, Log, TEXT("========================================\n"));

    // TODO: Add level transition logic here
    // For now this just logs to console in green (via LogTemp category)
}
