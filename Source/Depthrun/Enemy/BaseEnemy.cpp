// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "BaseEnemy.h"
#include "Core/DepthrunLogChannels.h"
#include "Combat/BaseProjectile.h"
#include "Combat/RangedWeapon.h"
#include "Components/CapsuleComponent.h"
#include "EnemyHealthComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Player/DepthrunCharacter.h"
#include "Components/WidgetComponent.h"
#include "UI/EnemyNumberWidget.h"
#include "PaperFlipbookComponent.h"

// FSM includes
#include "FSM/FSMComponent.h"
#include "FSM/States/FSMState_Attack.h"
#include "FSM/States/FSMState_Chase.h"
#include "FSM/States/FSMState_Flank.h"
#include "FSM/States/FSMState_Idle.h"
#include "FSM/States/FSMState_Retreat.h"

ABaseEnemy::ABaseEnemy() {
  PrimaryActorTick.bCanEverTick = true;

  HealthComponent =
      CreateDefaultSubobject<UEnemyHealthComponent>(TEXT("HealthComponent"));
  FSMComponent = CreateDefaultSubobject<UFSMComponent>(TEXT("FSMComponent"));

  EnemyType = EEnemyType::Melee;

  // ─── Capsule: flat for top-down 2D ────────────────────────────────────
  if (GetCapsuleComponent()) {
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    // Reduced to 8.f to prevent penetration issues and floor clipping
    // Similar to player capsule for consistent 2D collision behavior
    GetCapsuleComponent()->SetCapsuleHalfHeight(8.f);
    GetCapsuleComponent()->SetCapsuleRadius(14.f);
  }

  // ─── Sprite: lie flat on XY, no collision ─────────────────────────────
  if (GetSprite()) {
    GetSprite()->SetRelativeRotation(EnemySpriteRotationOffset);
    GetSprite()->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    GetSprite()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }

  // ─── Movement: Flying mode (same reason as player — avoids floor-loss
  // sliding) ─
  if (UCharacterMovementComponent* CMC = GetCharacterMovement())
  {
    CMC->GravityScale = 0.f;
    CMC->bOrientRotationToMovement = false;
    CMC->SetMovementMode(MOVE_Flying);
    // Lock DefaultLandMovementMode so physics cannot restore a wrong mode after teleport.
    CMC->DefaultLandMovementMode = MOVE_Flying;
    CMC->DefaultWaterMovementMode = MOVE_Flying;
    // CRITICAL: Enemies are driven by FSM/Adaptive components, not by an AIController.
    // Without a Controller, CMC's PerformMovement skips physics entirely
    // (treats the pawn as a network-remote one). This flag forces CMC to run
    // physics anyway, so AddMovementInput actually produces velocity.
    CMC->bRunPhysicsWithNoController = true;
    // NOTE: MaxFlySpeed is NOT set here — BeginPlay syncs it from MoveSpeed property.
    // Hardcoding a value here would silently override per-Blueprint MoveSpeed settings.
    // High deceleration to stop instantly — top-down 2D has no slide/inertia.
    CMC->BrakingDecelerationFlying = 8192.f;
    // NOTE: bConstrainToPlane is intentionally NOT set in the constructor.
    // It is enabled in SetLockedZ() after the correct Z plane origin is known.
    // Setting it here (with PlaneOrigin=0,0,0) would cause CMC to snap the enemy
    // to Z=0 every tick before SetLockedZ is called, causing collision with the floor.
  }

  // ─── Number widget component (shows #N label in world space) ──────────
  NumberWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("NumberWidgetComp"));
  NumberWidgetComp->SetupAttachment(RootComponent);
  NumberWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
  NumberWidgetComp->SetRelativeRotation(FRotator(90.f, 0.f, -90.f));
  NumberWidgetComp->SetWidgetSpace(EWidgetSpace::World);
  NumberWidgetComp->SetDrawSize(FVector2D(60.f, 30.f));
  NumberWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  NumberWidgetComp->SetVisibility(false); // hidden until index is assigned
}

void ABaseEnemy::BeginPlay() {
  Super::BeginPlay();

  UCharacterMovementComponent* CMC = GetCharacterMovement();

  // Stage 12: Integrity check. SafeDistance must be less than DetectionRange, 
  // otherwise the enemy retreats into "Idle" state and never comes back.
  SafeDistance = FMath::Clamp(SafeDistance, 50.f, DetectionRange * 0.8f);

  // ─── Weapon Spawning ──────────────────────────────────────────────────
  if (WeaponClass) {
    FActorSpawnParameters P;
    P.Owner = this;
    P.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass,
                                                        GetActorTransform(), P);
    if (SpawnedWeapon) {
      SpawnedWeapon->AttachToComponent(
          GetRootComponent(),
          FAttachmentTransformRules::SnapToTargetIncludingScale);

      // Commercial Fix: Explicitly pass the projectile class from the enemy to
      // the weapon. This solves the issue where the spawned weapon instance
      // might have a NULL ProjectileClass if it wasn't set in the class
      // defaults.
      if (ARangedWeapon *RW = Cast<ARangedWeapon>(SpawnedWeapon)) {
        if (ProjectileClass) {
          RW->ProjectileClass = ProjectileClass;
        }
      }
    }
  }

  // ─── Health callbacks ─────────────────────────────────────────────────
  if (HealthComponent) {
    HealthComponent->OnDeath.AddDynamic(this, &ABaseEnemy::OnDeath);
  }

  // ─── Player death: enemy dies with the player ─────────────────────────
  if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(
          UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
  {
      Player->OnPlayerDeath.AddDynamic(this, &ABaseEnemy::OnPlayerDied);
  }

  // ─── Movement speed from config ───────────────────────────────────────
  if (CMC)
  {
    CMC->MaxFlySpeed  = MoveSpeed;
    CMC->MaxWalkSpeed = MoveSpeed;
  }

  // ─── FSM setup: register all 5 states ─────────────────────────────────
  if (FSMComponent) {
    FSMComponent->RegisterState(EFSMStateType::Idle,
                                NewObject<UFSMState_Idle>(this));
    FSMComponent->RegisterState(EFSMStateType::Chase,
                                NewObject<UFSMState_Chase>(this));

    // Attack state: inherit cooldown from enemy config.
    UFSMState_Attack *AttackState = NewObject<UFSMState_Attack>(this);
    AttackState->AttackCooldown = AttackCooldown;
    FSMComponent->RegisterState(EFSMStateType::Attack, AttackState);

    FSMComponent->RegisterState(EFSMStateType::Retreat,
                                NewObject<UFSMState_Retreat>(this));
    FSMComponent->RegisterState(EFSMStateType::Flank,
                                NewObject<UFSMState_Flank>(this));

    // Start in Idle.
    FSMComponent->TransitionTo(EFSMStateType::Idle);
  }

  // Force an initial flipbook so the sprite is never blank on first frame.
  // Blueprint CDO sets FB_Idle etc., but the first Tick runs before any
  // engine-side BP default propagation completes in some edge cases.
  if (GetSprite() && !GetSprite()->GetFlipbook())
  {
    UPaperFlipbook* Fallback = FB_Idle ? FB_Idle : (FB_Walk ? FB_Walk : FB_Attack);
    if (Fallback)
    {
      GetSprite()->SetFlipbook(Fallback);
    }
  }
  if (GetSprite()) GetSprite()->SetVisibility(true, true);

  OnSpawned();
}

void ABaseEnemy::SetLockedZ(float InZ)
{
  if (bIsDead) return;

  FVector Loc = GetActorLocation();
  Loc.Z = InZ;
  SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);

  if (UCharacterMovementComponent* CMC = GetCharacterMovement())
  {
    // Now that we know the correct Z, enable the plane constraint.
    // Done here (not in constructor) to avoid CMC snapping enemy to Z=0
    // before the correct plane origin is set.
    CMC->SetPlaneConstraintNormal(FVector(0.f, 0.f, 1.f));
    CMC->SetPlaneConstraintOrigin(FVector(0.f, 0.f, InZ));
    CMC->bConstrainToPlane = true;
    CMC->bSnapToPlaneAtStart = false;
  }
}

void ABaseEnemy::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  UpdateAnimation();
}

void ABaseEnemy::PerformMeleeAttack() {
  ACharacter *Player = UGameplayStatics::GetPlayerCharacter(this, 0);
  if (!IsValid(Player))
    return;

  const float Dist =
      FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
  if (Dist > MeleeAttackRange)
    return; // sanity check: don't hit at range

  Player->TakeDamage(AttackDamage, FDamageEvent(), nullptr, this);
}

float ABaseEnemy::TakeDamage(float DamageAmount,
                             FDamageEvent const &DamageEvent,
                             AController *EventInstigator,
                             AActor *DamageCauser) {
  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage > 0.f && !bIsDead) {
    bIsHitAnimationActive = true;
    GetWorldTimerManager().SetTimer(
        HitAnimationTimer, this, &ABaseEnemy::ResetHitAnimation, 0.25f, false);
  }

  return ActualDamage;
}

void ABaseEnemy::ResetHitAnimation() { bIsHitAnimationActive = false; }

void ABaseEnemy::OnSpawned() {
  UE_LOG(LogTemp, Log, TEXT("ABaseEnemy::OnSpawned → %s"), *GetName());
}

void ABaseEnemy::OnKilled() {
  if (bIsDead)
    return;
  bIsDead = true;

  // Stop brain (FSM)
  if (FSMComponent) {
    FSMComponent->Deactivate();
  }

  // Stop updating movement
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
  }

  if (GetCapsuleComponent()) {
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }

  // Spawn death VFX
  if (NS_Death) {
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Death, GetActorLocation(),
        FRotator::ZeroRotator, FVector(1.f), true, true,
        ENCPoolMethod::AutoRelease);
  }

  // Play death animation instead of hiding immediately
  if (FB_Death && GetSprite()) {
    GetSprite()->SetFlipbook(FB_Death);
    GetSprite()->SetLooping(false);
    GetSprite()->Play();
  } else if (GetSprite()) {
    GetSprite()->SetHiddenInGame(true);
  }

  SetLifeSpan(1.0f);
}

void ABaseEnemy::OnDeath() { OnKilled(); }

void ABaseEnemy::OnPlayerDied()
{
    // Instantly kill this enemy when the player dies.
    // Prevent double-trigger if already dead.
    if (bIsDead) return;
    if (HealthComponent)
    {
        HealthComponent->ApplyDamage(HealthComponent->GetMaxHP() * 10.f);
    }
}

void ABaseEnemy::AssignDebugIndex(int32 Index)
{
    EnemyDebugIndex = Index;
    if (!NumberWidgetComp || !EnemyNumberWidgetClass) return;

    NumberWidgetComp->SetWidgetClass(EnemyNumberWidgetClass);
    NumberWidgetComp->SetVisibility(true);
    NumberWidgetComp->InitWidget(); // ensure widget is created before GetWidget() call

    if (UEnemyNumberWidget* W = Cast<UEnemyNumberWidget>(NumberWidgetComp->GetWidget()))
    {
        W->SetEnemyIndex(Index);
    }
}

void ABaseEnemy::ClearDebugIndex()
{
    EnemyDebugIndex = -1;
    if (NumberWidgetComp)
    {
        NumberWidgetComp->SetVisibility(false);
    }
}

void ABaseEnemy::UpdateAnimation() {
  if (!GetSprite() || bIsDead)
    return;

  // Pick flipbook: Hit/Attack anims take priority, then move/idle.
  UPaperFlipbook *DesiredFB = nullptr;

  const bool bAttacking = FSMComponent && FSMComponent->GetCurrentStateType() ==
                                               EFSMStateType::Attack;

  if (bIsHitAnimationActive && FB_Hit) {
    DesiredFB = FB_Hit;
  } else if (bAttacking) {
    DesiredFB = FB_Attack ? FB_Attack : FB_Idle;
  } else if (GetVelocity().SizeSquared() > 10.f) {
    DesiredFB = FB_Walk ? FB_Walk : FB_Idle;
  } else {
    DesiredFB = FB_Idle;
  }

  if (DesiredFB && GetSprite()->GetFlipbook() != DesiredFB) {
    GetSprite()->SetFlipbook(DesiredFB);
  }

  // Mirror sprite horizontally based on lateral movement direction.
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

FVector ABaseEnemy::GetSeparationSteering() const
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<ABaseEnemy*>(this));
	TArray<AActor*> OutActors;

    // Larger radius for better spacing (100 units)
	UKismetSystemLibrary::SphereOverlapActors(
		this, GetActorLocation(), 100.f, ObjectTypes, ABaseEnemy::StaticClass(), IgnoreActors, OutActors);

    FVector Steer = FVector::ZeroVector;
    for (AActor* Ally : OutActors)
    {
        if (Ally)
        {
            FVector Diff = GetActorLocation() - Ally->GetActorLocation();
            float Dist = Diff.Size();
            if (Dist > 0.01f)
            {
                Steer += Diff.GetSafeNormal() / Dist; // Stronger push if closer
            }
        }
    }
    return Steer.GetSafeNormal2D();
}
