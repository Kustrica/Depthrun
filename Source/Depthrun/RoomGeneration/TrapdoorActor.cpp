// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "TrapdoorActor.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Player/DepthrunCharacter.h"

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

    // Disable overlap for 3 seconds so player can't accidentally trigger immediately
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GetWorldTimerManager().SetTimer(ActivationTimer, this, &ATrapdoorActor::EnableOverlap, 3.0f, false);
}

void ATrapdoorActor::EnableOverlap()
{
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATrapdoorActor::OnTrapdoorOverlap);
    UE_LOG(LogTemp, Log, TEXT("[Trapdoor] Hatch activated - player can now use it."));
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

    UE_LOG(LogTemp, Log, TEXT("[Trapdoor] Player stepped on exit hatch — showing victory screen."));

    // Disable collision so it can't fire twice
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(OverlapPawn);
    if (!Player)
        return;

    // Freeze player movement
    if (UCharacterMovementComponent* Move = Player->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }
    Player->ConsumeMovementInputVector();

    // Show victory screen (handles diamonds, input lock, widget spawn)
    Player->ShowVictoryScreen();
}
