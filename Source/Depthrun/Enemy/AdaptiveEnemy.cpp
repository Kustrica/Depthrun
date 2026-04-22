// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveEnemy.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "DepthrunLogChannels.h"
#include "Enemy/EnemyHealthComponent.h"
#include "FSM/FSMComponent.h"
#include "FSM/States/FSMState_Attack.h"
#include "FSM/States/FSMState_Chase.h"
#include "FSM/States/FSMState_Flank.h"
#include "FSM/States/FSMState_Idle.h"
#include "FSM/States/FSMState_Retreat.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"
#include "Player/PlayerActionTracker.h"
#include "PaperFlipbookComponent.h"

AAdaptiveEnemy::AAdaptiveEnemy() {
  EnemyType = EEnemyType::Adaptive;

  AdaptiveComp = CreateDefaultSubobject<UAdaptiveBehaviorComponent>(
      TEXT("AdaptiveBehaviorComponent"));
}

void AAdaptiveEnemy::BeginPlay() {
  Super::BeginPlay();

  // 1. Register FSM states
  if (FSMComponent) {
    FSMComponent->RegisterState(EFSMStateType::Idle,
                                NewObject<UFSMState_Idle>(this));
    FSMComponent->RegisterState(EFSMStateType::Chase,
                                NewObject<UFSMState_Chase>(this));

    UFSMState_Attack *Atk = NewObject<UFSMState_Attack>(this);
    Atk->AttackCooldown = AttackCooldown;
    FSMComponent->RegisterState(EFSMStateType::Attack, Atk);

    FSMComponent->RegisterState(EFSMStateType::Retreat,
                                NewObject<UFSMState_Retreat>(this));
    FSMComponent->RegisterState(EFSMStateType::Flank,
                                NewObject<UFSMState_Flank>(this));

    // Start in Idle
    FSMComponent->TransitionTo(EFSMStateType::Idle);
  }

  // 2. Subscribe to Player Actions (for Memory and Pattern Recognition)
  ADepthrunCharacter *Player = Cast<ADepthrunCharacter>(
      UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
  if (Player && Player->GetActionTracker() && AdaptiveComp) {
    Player->GetActionTracker()->OnPlayerAction.AddDynamic(
        AdaptiveComp, &UAdaptiveBehaviorComponent::HandlePlayerAction);
  }

  // 3. Subscribe to Health delegates for Reward Signal (online adaptation)
  if (HealthComponent && AdaptiveComp) {
    // reward = -1 when taking damage
    HealthComponent->OnHealthChanged.AddDynamic(
        this, &AAdaptiveEnemy::HandleHealthChanged);
  }

  UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] Initialized: %s"),
         *GetName());
}

void AAdaptiveEnemy::HandleHealthChanged(float OldHP, float NewHP,
                                         float MaxHP) {
  if (NewHP < OldHP && AdaptiveComp) {
    AdaptiveComp->OnDamageTaken();
  }
}

void AAdaptiveEnemy::PerformMeleeAttack() {
  Super::PerformMeleeAttack();

  if (AdaptiveComp) {
    AdaptiveComp->OnDamageDealt();
  }
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
