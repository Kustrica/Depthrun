// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "RoomTransitionVolume.h"
#include "RoomGeneratorSubsystem.h"
#include "Components/BoxComponent.h"
#include "Player/DepthrunCharacter.h"

ARoomTransitionVolume::ARoomTransitionVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(64.f, 64.f, 64.f));
	RootComponent = TriggerBox;
}

void ARoomTransitionVolume::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARoomTransitionVolume::OnBeginOverlap);
}

void ARoomTransitionVolume::OnBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!Cast<ADepthrunCharacter>(OtherActor)) return;
	if (URoomGeneratorSubsystem* Gen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
	{
		Gen->OnPlayerEnteredTransition(OwnerRoom, ExitIndex);
	}
}
