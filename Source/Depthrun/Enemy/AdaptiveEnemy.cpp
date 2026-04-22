// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveEnemy.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "DepthrunLogChannels.h"
#include "FSM/FSMComponent.h"

AAdaptiveEnemy::AAdaptiveEnemy() {
  EnemyType = EEnemyType::Adaptive;

  AdaptiveComp = CreateDefaultSubobject<UAdaptiveBehaviorComponent>(
      TEXT("AdaptiveBehaviorComponent"));
}

void AAdaptiveEnemy::BeginPlay() {
  Super::BeginPlay();
  UE_LOG(LogAdaptiveBehavior, Log,
         TEXT("[AdaptiveEnemy] BeginPlay — stub, full init in Stage 7"));
  // TODO (Stage 7): register FSM states, subscribe to PlayerActionTracker, wire
  // reward delegates
}

void AAdaptiveEnemy::OnKilled() {
  Super::OnKilled();
  // Stop evaluation timer via AdaptiveComp EndPlay (handled automatically)
  UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] Killed"));
}

void AAdaptiveEnemy::UpdateAnimation() {
  if (!GetSprite() || bIsDead)
    return;

  // If in ranged mode, swap the pointers temporarily or just logic here.
  // We can't easily swap pointers because they are UPROPERTY.
  // So we override the logic.

  UPaperFlipbook *DesiredFB = nullptr;
  const bool bAttacking = FSMComponent && FSMComponent->GetCurrentStateType() ==
                                               EFSMStateType::Attack;

  if (bIsHitAnimationActive && FB_Hit) {
    DesiredFB = FB_Hit;
  } else if (bIsRangedMode) {
    if (bAttacking) {
      DesiredFB = FB_Attack_Ranged ? FB_Attack_Ranged : FB_Idle_Ranged;
    } else if (GetVelocity().SizeSquared() > 10.f) {
      DesiredFB = FB_Walk_Ranged ? FB_Walk_Ranged : FB_Idle_Ranged;
    } else {
      DesiredFB = FB_Idle_Ranged;
    }
  } else {
    // Melee mode (base)
    if (bAttacking) {
      DesiredFB = FB_Attack ? FB_Attack : FB_Idle;
    } else if (GetVelocity().SizeSquared() > 10.f) {
      DesiredFB = FB_Walk ? FB_Walk : FB_Idle;
    } else {
      DesiredFB = FB_Idle;
    }
  }

  if (DesiredFB && GetSprite()->GetFlipbook() != DesiredFB) {
    GetSprite()->SetFlipbook(DesiredFB);
  }

  // Mirror sprite horizontally (same as base)
  if (GetVelocity().SizeSquared() > 10.f) {
    const FVector V = GetVelocity();
    if (FMath::Abs(V.Y) > 0.1f) {
      FVector Scale = GetSprite()->GetRelativeScale3D();
      Scale.X = (V.Y < 0.f) ? -1.f : 1.f;
      GetSprite()->SetRelativeScale3D(Scale);
    }
  }
}
