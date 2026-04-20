// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveEnemy.h"
#include "FSM/FSMComponent.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "DepthrunLogChannels.h"

AAdaptiveEnemy::AAdaptiveEnemy()
{
	EnemyType = EEnemyType::Adaptive;

	AdaptiveComp = CreateDefaultSubobject<UAdaptiveBehaviorComponent>(TEXT("AdaptiveBehaviorComponent"));
}

void AAdaptiveEnemy::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] BeginPlay — stub, full init in Stage 7"));
	// TODO (Stage 7): register FSM states, subscribe to PlayerActionTracker, wire reward delegates
}

void AAdaptiveEnemy::OnKilled()
{
	Super::OnKilled();
	// Stop evaluation timer via AdaptiveComp EndPlay (handled automatically)
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] Killed"));
}
