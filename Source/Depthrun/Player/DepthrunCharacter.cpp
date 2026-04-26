// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunCharacter.h"
#include "Combat/BaseWeapon.h"
#include "Combat/MeleeWeapon.h"
#include "Core/DepthrunLogChannels.h"
#include "Items/RunItemInventory.h"
#include "PlayerActionTracker.h"
#include "PlayerCombatComponent.h"
#include "PlayerMovementConfig.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "UI/DepthrunHUD.h"

// ─── Timer handles for dash stop and attack reset ─────────────────────────
namespace {
// Duration after LaunchCharacter until we zero the velocity to kill sliding
constexpr float DashStopDelay = 0.12f;
} // namespace

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

  // Commercial Fix: Disable ALL post-process effects that cause artifacts in 2D
  // Paper2D
  // 1. Motion Blur
  FollowCamera->PostProcessSettings.bOverride_MotionBlurAmount = true;
  FollowCamera->PostProcessSettings.MotionBlurAmount = 0.f;
  // 2. Depth of Field (disable completely - keep sprite sharp at all times)
  FollowCamera->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
  FollowCamera->PostProcessSettings.DepthOfFieldFstop = 32.f;
  // 3. Anti-Aliasing: Note: This must be set in Project Settings -> Rendering
  // -> Anti-Aliasing Method = None (In modern UE5, AA method is a global
  // setting and cannot be overridden per-camera in FPostProcessSettings)
  // 4. Lumen Global Illumination: the MAIN cause of ghosting/blur with
  // lighting!
  FollowCamera->PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod =
      true;
  FollowCamera->PostProcessSettings.DynamicGlobalIlluminationMethod =
      EDynamicGlobalIlluminationMethod::None;
  // 5. Lumen Reflections
  FollowCamera->PostProcessSettings.bOverride_ReflectionMethod = true;
  FollowCamera->PostProcessSettings.ReflectionMethod = EReflectionMethod::None;
  // 6. Bloom (can cause halo artifacts on bright sprites)
  FollowCamera->PostProcessSettings.bOverride_BloomIntensity = true;
  FollowCamera->PostProcessSettings.BloomIntensity = 0.f;
  // 7. Ambient Occlusion (irrelevant in 2D, causes darkening)
  FollowCamera->PostProcessSettings.bOverride_AmbientOcclusionIntensity = true;
  FollowCamera->PostProcessSettings.AmbientOcclusionIntensity = 0.f;

  // ─── Player components ─────────────────────────────────────────────────
  CombatComponent =
      CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComponent"));
  ActionTracker =
      CreateDefaultSubobject<UPlayerActionTracker>(TEXT("ActionTracker"));
  ItemInventory =
      CreateDefaultSubobject<URunItemInventory>(TEXT("ItemInventory"));

  // ─── Movement (top-down, sharp 2D feel) ───────────────────────────────
  GetCharacterMovement()->bOrientRotationToMovement = false;
  GetCharacterMovement()->GravityScale = 0.f;

  // Sharp movement: high acceleration and braking to prevent "ice effect"
  // CRITICAL FIX: Use MOVE_Flying, not Walking. In top-down gravity=0 games,
  // Walking mode loses floor contact after LaunchCharacter (dash), switching to
  // Falling mode which has BrakingDecelerationFalling=0 by default — causing
  // infinite sliding.
  GetCharacterMovement()->MaxFlySpeed = 450.f;
  GetCharacterMovement()->MaxWalkSpeed = 450.f; // fallback
  GetCharacterMovement()->MaxAcceleration = 20480.f;
  GetCharacterMovement()->BrakingDecelerationFlying =
      20480.f; // Flying mode uses this
  GetCharacterMovement()->BrakingDecelerationWalking = 20480.f; // fallback
  GetCharacterMovement()->BrakingDecelerationFalling =
      20480.f; // in case of state transition
  GetCharacterMovement()->GroundFriction = 12.f;
  GetCharacterMovement()->BrakingFrictionFactor = 1.f;
  GetCharacterMovement()->bUseSeparateBrakingFriction = true;
  GetCharacterMovement()->BrakingFriction = 10.f;

  GetCharacterMovement()->SetMovementMode(MOVE_Flying);

  // ─── Capsule: small for 2D top-down ───────────────────────────────────
  GetCapsuleComponent()->SetCapsuleHalfHeight(16.f);
  GetCapsuleComponent()->SetCapsuleRadius(12.f);

  // Commercial Fix: Rotate sprite to lie flat on XY plane for top-down view
  // Aligned via SpriteRotationOffset property to allow manual adjustment in BP.
  if (GetSprite()) {
    GetSprite()->SetRelativeRotation(SpriteRotationOffset);
  }
  GetCapsuleComponent()->SetCapsuleRadius(12.f);

  // Sprite faces camera (no auto-rotation from movement)
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;
}

void ADepthrunCharacter::BeginPlay() {
  Super::BeginPlay();

  CurrentHP = MaxHP;
  bCanDash = true;

  // ─── Apply Movement Configuration ─────────────────────────────────────
  if (MovementConfig) {
    UCharacterMovementComponent *MoveComp = GetCharacterMovement();
    // Use MaxWalkSpeed for Flying mode's MaxFlySpeed too
    MoveComp->MaxFlySpeed = MovementConfig->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = MovementConfig->MaxWalkSpeed;
    MoveComp->MaxAcceleration = MovementConfig->MaxAcceleration;
    // Apply the BrakingDeceleration to ALL modes so dash never gets stuck in
    // wrong mode
    MoveComp->BrakingDecelerationFlying =
        MovementConfig->BrakingDecelerationWalking;
    MoveComp->BrakingDecelerationWalking =
        MovementConfig->BrakingDecelerationWalking;
    MoveComp->BrakingDecelerationFalling =
        MovementConfig->BrakingDecelerationWalking;
    MoveComp->GroundFriction = MovementConfig->GroundFriction;
    MoveComp->BrakingFrictionFactor = MovementConfig->BrakingFrictionFactor;
    MoveComp->bUseSeparateBrakingFriction =
        MovementConfig->bUseSeparateBrakingFriction;
    MoveComp->BrakingFriction = MovementConfig->BrakingFriction;

    DashImpulse = MovementConfig->DashImpulse;
    DashCooldown = MovementConfig->DashCooldown;
  }

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

  // ─── Spawn both weapon slots ───────────────────────────────────────────
  auto SpawnWeapon = [this](TSubclassOf<ABaseWeapon> WClass) -> ABaseWeapon * {
    if (!WClass)
      return nullptr;
    FActorSpawnParameters P;
    P.Owner = this;
    P.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return GetWorld()->SpawnActor<ABaseWeapon>(WClass, GetActorTransform(), P);
  };

  SpawnedWeapon1 = SpawnWeapon(WeaponSlotClass1);
  if (SpawnedWeapon1)
    SpawnedWeapon1->AttachToComponent(
        GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetIncludingScale);

  SpawnedWeapon2 = SpawnWeapon(WeaponSlotClass2);
  if (SpawnedWeapon2)
    SpawnedWeapon2->AttachToComponent(
        GetRootComponent(),
        FAttachmentTransformRules::SnapToTargetIncludingScale);

  // Commercial Fix: Hide and disable collisions for both weapons initially
  auto DisableWeapon = [](ABaseWeapon *W) {
    if (W) {
      W->SetActorHiddenInGame(true);
      W->SetActorEnableCollision(false);
    }
  };
  DisableWeapon(SpawnedWeapon1);
  DisableWeapon(SpawnedWeapon2);

  // Start with Slot 1 (Sword) active
  SwitchToWeaponSlot(1);

  if (!SpawnedWeapon1 && !SpawnedWeapon2) {
    UE_LOG(
        LogDepthrun, Warning,
        TEXT("[Character] No weapon slots assigned in BP_DepthrunCharacter!"));
  }

  UE_LOG(LogDepthrun, Log,
         TEXT("ADepthrunCharacter::BeginPlay — player ready, HP=%.0f"), MaxHP);

  if (APlayerController* PC = Cast<APlayerController>(GetController()))
  {
      if (ADepthrunHUD* HUD = Cast<ADepthrunHUD>(PC->GetHUD()))
      {
          HUD->UpdatePlayerHP(CurrentHP, MaxHP);
      }
  }
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
    if (IA_Attack)
      EIC->BindAction(IA_Attack, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleAttack);
    if (IA_Dash)
      EIC->BindAction(IA_Dash, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleDash);
    if (IA_SwitchSlot1)
      EIC->BindAction(IA_SwitchSlot1, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleSwitchSlot1);
    if (IA_SwitchSlot2)
      EIC->BindAction(IA_SwitchSlot2, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleSwitchSlot2);
  }

  // --- Debug Bindings ---
  // Using Y and U as requested (F4/F5 are used by Engine)
  PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, this,
                                &ADepthrunCharacter::ToggleGodMode);
  PlayerInputComponent->BindKey(EKeys::U, IE_Pressed, this,
                                &ADepthrunCharacter::ToggleSuperAttack);
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
    UE_LOG(LogTemp, Warning,
           TEXT("[HandleAttack] Failed: No weapon equipped. Assign "
                "DefaultWeaponClass in BP_DepthrunCharacter!"));
    return;
  }

  // --- Super Attack Debug Mode ---
  float OriginalDamage = 0.f;
  float OriginalRange = 1.0f;
  AMeleeWeapon *MeleeW = Cast<AMeleeWeapon>(CombatComponent->CurrentWeapon);

  if (bSuperAttack && CombatComponent->CurrentWeapon) {
    OriginalDamage = CombatComponent->CurrentWeapon->BaseDamage;
    CombatComponent->CurrentWeapon->BaseDamage = 9999.f;

    if (MeleeW) {
      OriginalRange = MeleeW->GetRangeMultiplier();
      MeleeW->SetRangeMultiplier(5.0f); // 5x range for melee
    }
  }

  // Tell the weapon which direction to fire before calling Attack()
  CombatComponent->CurrentWeapon->SetFireDirection(GetFireDirection());
  CombatComponent->Attack();

  // Reset Super Attack values after firing (so it doesn't permanently ruin weapon stats)
  if (bSuperAttack && CombatComponent->CurrentWeapon) {
    CombatComponent->CurrentWeapon->BaseDamage = OriginalDamage;
    if (MeleeW) {
      MeleeW->SetRangeMultiplier(OriginalRange);
    }
  }

  // ── Play attack animation for the duration of the weapon cooldown ──────
  bIsAttacking = true;
  UpdateAnimation();

  const float CooldownDur =
      CombatComponent->CurrentWeapon->AttackCooldown > 0.05f
          ? CombatComponent->CurrentWeapon->AttackCooldown
          : 0.3f;
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
void ADepthrunCharacter::HandleSwitchSlot1(const FInputActionValue &) {
  SwitchToWeaponSlot(1);
}
void ADepthrunCharacter::HandleSwitchSlot2(const FInputActionValue &) {
  SwitchToWeaponSlot(2);
}

// ─────────────────────────── Weapon Slot Switch ────────────────────────────

void ADepthrunCharacter::SwitchToWeaponSlot(int32 SlotIndex) {
  // Commercial Fix: Allow switching even if it's the same slot to fix potential
  // desyncs, or at least ensure the correct weapon is visible/enabled.

  auto UpdateWeaponState = [](ABaseWeapon *W, bool bActive) {
    if (W) {
      W->SetActorHiddenInGame(!bActive);
      W->SetActorEnableCollision(bActive);
    }
  };

  if (SlotIndex == 1 && SpawnedWeapon1) {
    UpdateWeaponState(SpawnedWeapon1, true);
    UpdateWeaponState(SpawnedWeapon2, false);
    if (CombatComponent)
      CombatComponent->EquipWeapon(SpawnedWeapon1);
    if (ItemInventory)
      ItemInventory->ApplyToWeapon(SpawnedWeapon1);
    ActiveWeaponSlot = 1;
    UE_LOG(LogDepthrun, Log, TEXT("[Weapon] Switched to Slot 1 (Sword)"));
  } else if (SlotIndex == 2 && SpawnedWeapon2) {
    UpdateWeaponState(SpawnedWeapon1, false);
    UpdateWeaponState(SpawnedWeapon2, true);
    if (CombatComponent)
      CombatComponent->EquipWeapon(SpawnedWeapon2);
    if (ItemInventory)
      ItemInventory->ApplyToWeapon(SpawnedWeapon2);
    ActiveWeaponSlot = 2;
    UE_LOG(LogDepthrun, Log, TEXT("[Weapon] Switched to Slot 2 (Bow)"));
  }
  UpdateAnimation();
}

// ─────────────────────────── Dash ─────────────────────────────────────────

void ADepthrunCharacter::Dash() {
  if (!bCanDash || bIsDead)
    return;

  bCanDash = false;

  // Дэш в направлении движения; если стоим — по направлению взгляда
  const FVector DashDir = GetVelocity().IsNearlyZero()
                              ? GetFireDirection()
                              : GetVelocity().GetSafeNormal();

  UCharacterMovementComponent *MoveComp = GetCharacterMovement();
  const float Impulse =
      MovementConfig ? MovementConfig->DashImpulse : DashImpulse;
  const float Duration = MovementConfig ? MovementConfig->DashDuration : 0.15f;
  const float CancelFactor =
      MovementConfig ? MovementConfig->PostDashVelocityCancelFactor : 0.1f;

  // ── FIX: Отключаем торможение на время дэша, иначе BrakingDecelerationFlying
  //         гасит импульс в тот же тик, в котором он был применён (именно это
  //         вызывало «дёргание» камеры без реального перемещения персонажа).
  MoveComp->BrakingDecelerationFlying = 0.f;
  MoveComp->BrakingFriction = 0.f;
  MoveComp->GroundFriction = 0.f;

  // ── FIX: Снимаем ограничение MaxFlySpeed на время дэша, иначе импульс
  //         обрезается до 400 UU/s ещё до того, как CMC его применит.
  //         Продолжительность дэша контролируется таймером, а не скоростью.
  MoveComp->MaxFlySpeed = 9999.f;

  // Напрямую задаём Velocity — надёжнее, чем LaunchCharacter в MOVE_Flying,
  // потому что LaunchCharacter всё равно проходит через ConsumeInputVector()
  // и может быть частично погашен той же декелерацией в том же SubStep.
  MoveComp->Velocity = DashDir * Impulse;

  // ── Через DashDuration: восстанавливаем параметры торможения и гасим инерцию
  GetWorldTimerManager().SetTimer(
      DashStopTimer,
      [this, CancelFactor]() {
        UCharacterMovementComponent *MC = GetCharacterMovement();
        if (!MC)
          return;

        // Восстанавливаем параметры из конфига (или дефолты конструктора)
        const float BrakeDec = MovementConfig
                                   ? MovementConfig->BrakingDecelerationWalking
                                   : 20480.f;
        const float BrakeFric =
            MovementConfig ? MovementConfig->BrakingFriction : 10.f;
        const float GndFric =
            MovementConfig ? MovementConfig->GroundFriction : 12.f;
        const float FlySpeed =
            MovementConfig ? MovementConfig->MaxWalkSpeed : 450.f;

        MC->BrakingDecelerationFlying = BrakeDec;
        MC->BrakingFriction = BrakeFric;
        MC->GroundFriction = GndFric;
        MC->MaxFlySpeed = FlySpeed;

        // Гасим остаточную скорость после дэша
        MC->Velocity *= CancelFactor;
        if (CancelFactor <= 0.05f) {
          MC->StopMovementImmediately();
        }
      },
      Duration, false);

  // Записываем действие в память врагов
  ActionTracker->RecordAction(EPlayerActionType::Dash, GetActorLocation());

  UE_LOG(LogDepthrun, Verbose,
         TEXT("ADepthrunCharacter::Dash dir=(%.1f,%.1f,%.1f) impulse=%.0f "
              "dur=%.2fs"),
         DashDir.X, DashDir.Y, DashDir.Z, Impulse, Duration);

  const float Cooldown =
      MovementConfig ? MovementConfig->DashCooldown : DashCooldown;
  GetWorldTimerManager().SetTimer(DashCooldownTimer, this,
                                  &ADepthrunCharacter::OnDashCooldownEnd,
                                  Cooldown, false);
}

void ADepthrunCharacter::OnDashCooldownEnd() { bCanDash = true; }

float ADepthrunCharacter::TakeDamage(float DamageAmount,
                                     FDamageEvent const &DamageEvent,
                                     AController *EventInstigator,
                                     AActor *DamageCauser) {
  if (DamageAmount <= 0.f || bIsDead || bGodMode)
    return 0.f;

  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);
  CurrentHP -= ActualDamage;

  if (ActualDamage > 0.f) {
    bIsHitAnimationActive = true;
    GetWorldTimerManager().SetTimer(HitAnimTimer, this,
                                    &ADepthrunCharacter::ResetHitAnimation,
                                    0.2f, false);
  }

  UE_LOG(LogDepthrun, Log,
         TEXT("[Player] Took %.1f damage from %s. Remaining HP: %.1f"), ActualDamage,
         DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"), CurrentHP);

  if (CurrentHP <= 0.f) {
    Die();
  }

  if (APlayerController* PC = Cast<APlayerController>(GetController()))
  {
      if (ADepthrunHUD* HUD = Cast<ADepthrunHUD>(PC->GetHUD()))
      {
          HUD->UpdatePlayerHP(CurrentHP, MaxHP);
      }
  }

  UpdateAnimation();
  return ActualDamage;
}

void ADepthrunCharacter::Die() {
  if (bIsDead)
    return;
  bIsDead = true;

  UE_LOG(LogDepthrun, Error, TEXT("[Player] DIED!"));

  if (GetCharacterMovement())
    GetCharacterMovement()->StopMovementImmediately();
  SetActorEnableCollision(false);

  if (FB_Death && GetSprite()) {
    GetSprite()->SetFlipbook(FB_Death);
    GetSprite()->SetLooping(false);
    GetSprite()->Play();
  }
}

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
    return FVector(0.f, 1.f, 0.f); // In HandleMove, A/D (Input.X) -> World Y
  case EPlayerFacingDirection::Left:
    return FVector(0.f, -1.f, 0.f);
  case EPlayerFacingDirection::Up:
    return FVector(1.f, 0.f, 0.f); // In HandleMove, W/S (Input.Y) -> World X
  case EPlayerFacingDirection::Down:
    return FVector(-1.f, 0.f, 0.f);
  default:
    return FVector(1.f, 0.f, 0.f);
  }
}

void ADepthrunCharacter::UpdateAnimation() {
  if (!GetSprite() || bIsDead)
    return;

  UPaperFlipbook *Desired = nullptr;

  // ─── 1. Determine if we should FLIP the sprite (for Left direction) ────
  const bool bShouldFlip = (FacingDirection == EPlayerFacingDirection::Left);

  // Use Scale3D.X = -1 to flip the sprite properly in Paper2D
  FVector TargetScale = GetSprite()->GetRelativeScale3D();
  TargetScale.X = bShouldFlip ? -1.f : 1.f;
  if (GetSprite()->GetRelativeScale3D().X != TargetScale.X) {
    GetSprite()->SetRelativeScale3D(TargetScale);
  }

  // ─── 2. Select appropriate Flipbook ──────────────────────────────────
  if (bIsHitAnimationActive && FB_Hit) {
    Desired = FB_Hit;
  } else if (bIsAttacking && CombatComponent &&
             IsValid(CombatComponent->CurrentWeapon)) {
    const bool bIsMelee =
        CombatComponent->CurrentWeapon->GetWeaponType() == EWeaponType::Melee;

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
FText ADepthrunCharacter::GetGodStatusText() const {
  return bGodMode ? FText::FromString(TEXT("GOD MODE: ON")) : FText::GetEmpty();
}

FText ADepthrunCharacter::GetSuperAttackStatusText() const {
  return bSuperAttack ? FText::FromString(TEXT("SUPER ATTACK: ON"))
                      : FText::GetEmpty();
}
