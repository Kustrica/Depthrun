// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunCharacter.h"
#include "Combat/BaseWeapon.h"
#include "Core/DepthrunLogChannels.h"
#include "PlayerActionTracker.h"
#include "PlayerCombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

// ─── Timer handles for dash stop and attack reset ─────────────────────────
namespace
{
	// Duration after LaunchCharacter until we zero the velocity to kill sliding
	constexpr float DashStopDelay = 0.12f;
}

ADepthrunCharacter::ADepthrunCharacter() {
  PrimaryActorTick.bCanEverTick = false;

  // ─── Top-down camera ───────────────────────────────────────────────────
  SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
  SpringArm->SetupAttachment(RootComponent);
  SpringArm->TargetArmLength = 700.f;
  SpringArm->SetRelativeRotation(
      FRotator(-90.f, 0.f, 0.f)); // look straight down
  SpringArm->bDoCollisionTest = false;
  SpringArm->bInheritPitch = false;
  SpringArm->bInheritYaw = false;
  SpringArm->bInheritRoll = false;

  FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
  FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
  FollowCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
  FollowCamera->OrthoWidth = 1024.f;

  // ─── Player components ─────────────────────────────────────────────────
  CombatComponent =
      CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComponent"));
  ActionTracker =
      CreateDefaultSubobject<UPlayerActionTracker>(TEXT("ActionTracker"));

  // ─── Movement (top-down, sharp 2D feel) ───────────────────────────────
  GetCharacterMovement()->bOrientRotationToMovement = false;
  GetCharacterMovement()->GravityScale = 0.f;

  // Sharp movement: high acceleration and braking to prevent "ice effect"
  GetCharacterMovement()->MaxWalkSpeed = 500.f;
  GetCharacterMovement()->MaxAcceleration = 50000.f;
  GetCharacterMovement()->BrakingDecelerationWalking = 50000.f;
  GetCharacterMovement()->GroundFriction = 10.f;
  GetCharacterMovement()->BrakingFrictionFactor = 2.f;

  GetCharacterMovement()->SetMovementMode(MOVE_Walking);

  // ─── Capsule: small for 2D top-down ───────────────────────────────────
  GetCapsuleComponent()->SetCapsuleHalfHeight(16.f);
  GetCapsuleComponent()->SetCapsuleRadius(12.f);

  // Sprite faces camera (no auto-rotation from movement)
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;
}

void ADepthrunCharacter::BeginPlay() {
  Super::BeginPlay();

  CurrentHP = MaxHP;

  // Register Enhanced Input mapping context
  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer())) {
      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
      }
    }
  }

  // --- Spawn & Equip Default Weapon ---
  if (DefaultWeaponClass) {
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ABaseWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(DefaultWeaponClass, GetActorTransform(), SpawnParams);
    if (SpawnedWeapon && CombatComponent) {
      CombatComponent->EquipWeapon(SpawnedWeapon);
      UE_LOG(LogDepthrun, Log, TEXT("Equipped DefaultWeaponClass %s"), *DefaultWeaponClass->GetName());
    }
  }

  UE_LOG(LogDepthrun, Log,
         TEXT("ADepthrunCharacter::BeginPlay — player ready, HP=%.0f"), MaxHP);
}

void ADepthrunCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent *EIC =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    if (IA_Move) {
      EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this,
                      &ADepthrunCharacter::HandleMove);
      EIC->BindAction(IA_Move, ETriggerEvent::Completed, this,
                      &ADepthrunCharacter::HandleMove);
    }
    if (IA_Attack) {
      EIC->BindAction(IA_Attack, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleAttack);
    }
    if (IA_Dash) {
      EIC->BindAction(IA_Dash, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleDash);
    }
  }
}

// ─────────────────────────── Input handlers ───────────────────────────────

void ADepthrunCharacter::HandleMove(const FInputActionValue &Value) {
  const FVector2D Input = Value.Get<FVector2D>();
  bIsMoving = !Input.IsNearlyZero();

  if (bIsMoving) {
    UpdateFacingDirection(Input);
    // Corrected top-down mapping:
    // W/S (Input.Y) moves along world X Axis (Screen Up/Down)
    // A/D (Input.X) moves along world Y Axis (Screen Left/Right)
    AddMovementInput(FVector(Input.Y, Input.X, 0.f));
  }

  UpdateAnimation();
}

void ADepthrunCharacter::HandleAttack(const FInputActionValue &Value) {
  if (!CombatComponent || !IsValid(CombatComponent->CurrentWeapon)) {
    UE_LOG(LogTemp, Warning, TEXT("[HandleAttack] Failed: No weapon equipped. Assign DefaultWeaponClass in BP_DepthrunCharacter!"));
    return;
  }

  // Tell the weapon which direction to fire before calling Attack()
  CombatComponent->CurrentWeapon->SetFireDirection(GetFireDirection());
  CombatComponent->Attack();

  // ── Play attack animation for the duration of the weapon cooldown ──────
  bIsAttacking = true;
  UpdateAnimation();

  const float CooldownDur = CombatComponent->CurrentWeapon->AttackCooldown > 0.05f ? CombatComponent->CurrentWeapon->AttackCooldown : 0.3f;
  GetWorldTimerManager().SetTimer(
      AttackAnimTimer,
      [this]() {
        bIsAttacking = false;
        UpdateAnimation();
      },
      CooldownDur, false);

  // Record action for enemy memory
  const EPlayerActionType ActionType =
      (CombatComponent->CurrentWeapon->GetWeaponType() == EWeaponType::Ranged)
          ? EPlayerActionType::Shot
          : EPlayerActionType::MeleeAttack;
  ActionTracker->RecordAction(ActionType, GetActorLocation());
}

void ADepthrunCharacter::HandleDash(const FInputActionValue &Value) { Dash(); }

// ─────────────────────────── Dash ─────────────────────────────────────────

void ADepthrunCharacter::Dash() {
  if (!bCanDash)
    return;

  bCanDash = false;

  // Dash in current facing direction; if standing still — use last facing
  const FVector DashDir = GetVelocity().IsNearlyZero()
                              ? GetFireDirection()
                              : GetVelocity().GetSafeNormal();

  LaunchCharacter(DashDir * DashImpulse, true, true);

  // ── Stop sliding: zero velocity shortly after the impulse ─────────────
  GetWorldTimerManager().SetTimer(
      DashStopTimer,
      [this]() {
        GetCharacterMovement()->StopMovementImmediately();
      },
      DashStopDelay, false);

  // Record dash for enemy memory
  ActionTracker->RecordAction(EPlayerActionType::Dash, GetActorLocation());

  UE_LOG(LogDepthrun, Verbose,
         TEXT("ADepthrunCharacter::Dash dir=(%.1f,%.1f,%.1f)"), DashDir.X,
         DashDir.Y, DashDir.Z);

  GetWorldTimerManager().SetTimer(DashCooldownTimer, this,
                                  &ADepthrunCharacter::OnDashCooldownEnd,
                                  DashCooldown, false);
}

void ADepthrunCharacter::OnDashCooldownEnd() { bCanDash = true; }

// ─────────────────────────── Helpers ──────────────────────────────────────

void ADepthrunCharacter::UpdateFacingDirection(const FVector2D &Input) {
  if (FMath::Abs(Input.X) >= FMath::Abs(Input.Y)) {
    FacingDirection = Input.X > 0.f ? EPlayerFacingDirection::Right
                                    : EPlayerFacingDirection::Left;
  } else {
    FacingDirection = Input.Y > 0.f ? EPlayerFacingDirection::Up
                                    : EPlayerFacingDirection::Down;
  }
}

FVector ADepthrunCharacter::GetFireDirection() const {
  switch (FacingDirection) {
  case EPlayerFacingDirection::Right:
    return FVector(1.f, 0.f, 0.f);
  case EPlayerFacingDirection::Left:
    return FVector(-1.f, 0.f, 0.f);
  case EPlayerFacingDirection::Up:
    return FVector(0.f, 1.f, 0.f);
  case EPlayerFacingDirection::Down:
    return FVector(0.f, -1.f, 0.f);
  default:
    return FVector(1.f, 0.f, 0.f);
  }
}

void ADepthrunCharacter::UpdateAnimation() {
  UPaperFlipbook *Desired = nullptr;

  // ─── 1. Determine if we should FLIP the sprite (for Left direction) ────
  const bool bShouldFlip = (FacingDirection == EPlayerFacingDirection::Left);
  
  // Use Scale3D.X = -1 to flip the sprite properly in Paper2D
  FVector TargetScale = GetSprite()->GetRelativeScale3D();
  TargetScale.X = bShouldFlip ? -1.f : 1.f;
  if (GetSprite()->GetRelativeScale3D().X != TargetScale.X)
  {
      GetSprite()->SetRelativeScale3D(TargetScale);
  }

  // ─── 2. Select appropriate Flipbook ──────────────────────────────────
  if (bIsAttacking && CombatComponent && IsValid(CombatComponent->CurrentWeapon)) {
    const bool bIsMelee = CombatComponent->CurrentWeapon->GetWeaponType() == EWeaponType::Melee;
    
    switch (FacingDirection) {
    case EPlayerFacingDirection::Right:
    case EPlayerFacingDirection::Left: // Handled by flip
      Desired = bIsMelee ? FB_MeleeAttackRight : FB_RangedAttackRight;
      break;
    case EPlayerFacingDirection::Up:
      Desired = bIsMelee ? FB_MeleeAttackUp : FB_RangedAttackUp;
      break;
    case EPlayerFacingDirection::Down:
      Desired = bIsMelee ? FB_MeleeAttackDown : FB_RangedAttackDown;
      break;
    }
  } else if (bIsMoving) {
    switch (FacingDirection) {
    case EPlayerFacingDirection::Right:
    case EPlayerFacingDirection::Left: // Handled by flip
      Desired = FB_WalkRight;
      break;
    case EPlayerFacingDirection::Up:
      Desired = FB_WalkUp;
      break;
    case EPlayerFacingDirection::Down:
      Desired = FB_WalkDown;
      break;
    }
  } else {
    switch (FacingDirection) {
    case EPlayerFacingDirection::Right:
    case EPlayerFacingDirection::Left: // Handled by flip
      Desired = FB_IdleRight;
      break;
    case EPlayerFacingDirection::Up:
      Desired = FB_IdleUp;
      break;
    case EPlayerFacingDirection::Down:
      Desired = FB_IdleDown;
      break;
    }
  }

  if (Desired && GetSprite()->GetFlipbook() != Desired) {
    GetSprite()->SetFlipbook(Desired);
  }
}
