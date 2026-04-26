// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveEnemy.h"
#include "Enemy/BaseEnemy.h"
#include "Combat/BaseProjectile.h"
#include "Combat/RangedWeapon.h"
#include "Components/SphereComponent.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Core/DepthrunLogChannels.h"
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
  if (bIsRangedMode) {
    // Always use direct projectile spawn for ranged mode.
    // SpawnedWeapon is intentionally bypassed here — it is the melee weapon
    // and must not intercept ranged attacks.
    if (!ProjectileClass)
    {
      UE_LOG(LogAdaptiveBehavior, Warning,
             TEXT("[AdaptiveEnemy] %s bIsRangedMode=true but ProjectileClass is NULL — assign it in BP!"),
             *GetName());
      return;
    }

    UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] %s firing ranged projectile (ShotDelay=%.2f)"),
           *GetName(), ShotDelay);

    if (ShotDelay > 0.01f) {
      FTimerHandle Timer;
      GetWorldTimerManager().SetTimer(Timer, this, &AAdaptiveEnemy::ActuallyFire,
                                      ShotDelay, false);
    } else {
      ActuallyFire();
    }
  } else {
    Super::PerformMeleeAttack();
  }

  if (AdaptiveComp) {
    AdaptiveComp->OnDamageDealt();
  }
}

void AAdaptiveEnemy::ActuallyFire() {
  ACharacter *Player = UGameplayStatics::GetPlayerCharacter(this, 0);
  if (!IsValid(Player))
    return;

  const FVector EnemyLoc = GetActorLocation();
  const FVector PlayerLoc = Player->GetActorLocation();
  const FVector FireDir = (PlayerLoc - EnemyLoc).GetSafeNormal2D();

  const FVector SpawnLoc =
      EnemyLoc + (FireDir * MuzzleOffset) + FVector(0.f, 0.f, 10.0f);
  const FTransform SpawnTransform = FTransform(FireDir.Rotation(), SpawnLoc);

  UWorld *World = GetWorld();
  if (!World)
    return;

  ABaseProjectile *Projectile = World->SpawnActorDeferred<ABaseProjectile>(
      ProjectileClass, SpawnTransform, this, Cast<APawn>(this),
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

  if (!IsValid(Projectile))
    return;

  Projectile->CollisionSphere->IgnoreActorWhenMoving(this, true);
  Projectile->InitProjectile(FireDir, AttackDamage, this, ProjectileSpeed);
  Projectile->FinishSpawning(SpawnTransform);
}

void AAdaptiveEnemy::OnKilled() {
  // We set the correct death flipbook BEFORE calling Super::OnKilled
  // so that BaseEnemy's logic picks it up (though BaseEnemy uses FB_Death directly, 
  // so we actually need to swap FB_Death temporarily or handle it here).
  
  if (bIsRangedMode && FB_Death_Ranged)
  {
      // Swap FB_Death to the ranged version so Super::OnKilled uses the right one
      FB_Death = FB_Death_Ranged;
  }

  Super::OnKilled();
  UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveEnemy] Killed: cleaning up evaluation."));
}

void AAdaptiveEnemy::UpdateAnimation() {
  if (!GetSprite() || bIsDead)
    return;

  UPaperFlipbook *DesiredFB = nullptr;
  
  // Use FSM to check if we are in Attack state
  const bool bAttacking = FSMComponent && FSMComponent->GetCurrentStateType() == EFSMStateType::Attack;
  
  // Also check if we are in Retreat state but shooting (Hybrid Stage 12)
  bool bRetreatShooting = false;
  if (FSMComponent && FSMComponent->GetCurrentStateType() == EFSMStateType::Retreat && bIsRangedMode)
  {
      // We'll consider it "attacking" animation if we just fired (this is a bit of a hack without a timer, but works for demo)
      // Actually, let's just stick to Walk_Ranged for retreat for now to avoid flickering.
  }

  if (bIsHitAnimationActive) {
    if (bIsRangedMode) {
      DesiredFB = FB_Hit_Ranged ? FB_Hit_Ranged : FB_Hit;
    } else {
      DesiredFB = FB_Hit;
    }
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

  // Mirror sprite horizontally
  if (GetVelocity().SizeSquared() > 10.f) {
    const FVector V = GetVelocity();
    if (FMath::Abs(V.Y) > 0.1f) {
      FVector Scale = GetSprite()->GetRelativeScale3D();
      const float XAbs = FMath::Max(FMath::Abs(Scale.X), 1.0f);
      Scale.X = (V.Y < 0.f) ? -XAbs : XAbs;
      GetSprite()->SetRelativeScale3D(Scale);
    }
  }
}
