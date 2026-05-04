// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "DepthrunCharacter.h"
#include "Combat/BaseWeapon.h"
#include "Combat/MeleeWeapon.h"
#include "Core/DepthrunLogChannels.h"
#include "Data/DepthrunSaveSubsystem.h"
#include "Data/SQLiteManager.h"
#include "Audio/CombatMusicTrigger.h"
#include "Data/HubUpgradeTypes.h"
#include "Items/RunItemInventory.h"
#include "Items/RunItemCollection.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"
#include "PlayerActionTracker.h"
#include "PlayerCombatComponent.h"
#include "PlayerEconomy.h"
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
  PrimaryActorTick.bCanEverTick = true;

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
  CombatMusicTrigger =
      CreateDefaultSubobject<UCombatMusicTrigger>(TEXT("CombatMusicTrigger"));
  PlayerEconomy =
      CreateDefaultSubobject<UPlayerEconomy>(TEXT("PlayerEconomy"));

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

  // ─── Hard XY plane constraint — prevents Z drift in Flying mode ───────
  // Flying mode has no floor, so sweep collisions with doors/obstacles can
  // push the capsule upward. bConstrainToPlane locks all movement to the XY
  // plane at the engine level, so no per-frame Tick correction is needed.
  GetCharacterMovement()->bConstrainToPlane   = true;
  GetCharacterMovement()->bSnapToPlaneAtStart = true;
  GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, 0.f, 1.f));

  // ─── Capsule: small for 2D top-down ───────────────────────────────────
  GetCapsuleComponent()->SetCapsuleHalfHeight(4.f);
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
  RunStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

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

  SetActorHiddenInGame(false);
  if (UPaperFlipbookComponent* PlayerSprite = GetSprite()) {
    PlayerSprite->SetHiddenInGame(false);
    PlayerSprite->SetVisibility(true, true);
    PlayerSprite->MarkRenderStateDirty();
  }

  // Make sure an idle flipbook is bound on spawn — without this the sprite
  // stays on the (often empty) BP default until the first input tick, which
  // looked like the character was invisible right after entering the level.
  UpdateAnimation();
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
    if (IA_UsePotion)
      EIC->BindAction(IA_UsePotion, ETriggerEvent::Started, this,
                      &ADepthrunCharacter::HandleUsePotion);
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
  if (bIsDead) return;
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
  if (bIsDead) return;
  if (!CombatComponent || !IsValid(CombatComponent->CurrentWeapon)) {
    UE_LOG(LogTemp, Warning,
           TEXT("[HandleAttack] Failed: No weapon equipped. Assign "
                "DefaultWeaponClass in BP_DepthrunCharacter!"));
    return;
  }

  // --- Super Attack Debug Mode ---
  float OriginalDamage = 0.f;
  float OriginalRange = 1.0f;
  float OriginalWidth = 1.0f;
  float OriginalThickness = 1.0f;
  bool bOriginalOmniSwing = false;
  AMeleeWeapon *MeleeW = Cast<AMeleeWeapon>(CombatComponent->CurrentWeapon);

  if (bSuperAttack && CombatComponent->CurrentWeapon) {
    OriginalDamage = CombatComponent->CurrentWeapon->BaseDamage;
    CombatComponent->CurrentWeapon->BaseDamage = 9999.f;

    if (MeleeW) {
      OriginalRange = MeleeW->GetRangeMultiplier();
      OriginalWidth = MeleeW->GetWidthMultiplier();
      OriginalThickness = MeleeW->GetThicknessMultiplier();
      bOriginalOmniSwing = MeleeW->IsOmniSwing();
      MeleeW->SetRangeMultiplier(5.0f);
      MeleeW->SetWidthMultiplier(4.0f);
      MeleeW->SetThicknessMultiplier(5.0f); // cube-shaped zone: tall enough to hit all Z planes
      MeleeW->SetOmniSwing(true);
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
      MeleeW->SetWidthMultiplier(OriginalWidth);
      MeleeW->SetThicknessMultiplier(OriginalThickness);
      MeleeW->SetOmniSwing(bOriginalOmniSwing);
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

void ADepthrunCharacter::HandleDash(const FInputActionValue &Value) {
  if (bIsDead) return;
  Dash();
}
void ADepthrunCharacter::HandleSwitchSlot1(const FInputActionValue &) {
  SwitchToWeaponSlot(1);
}
void ADepthrunCharacter::HandleSwitchSlot2(const FInputActionValue &) {
  SwitchToWeaponSlot(2);
}

void ADepthrunCharacter::HandleUsePotion(const FInputActionValue &) {
  if (PlayerEconomy) {
    PlayerEconomy->UsePotion();
  }
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

void ADepthrunCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateChromaticAberration(DeltaSeconds);
}

void ADepthrunCharacter::TriggerChromaticAberration()
{
	// Reset hold timer — if already active, extend hold
	CA_HoldTimeRemaining = HitFX_HoldDuration;
	bCA_FadingOut = false;
}

void ADepthrunCharacter::UpdateChromaticAberration(float DeltaSeconds)
{
	if (!FollowCamera) return;
	if (CA_CurrentIntensity <= 0.f && bCA_FadingOut) return;

	if (!bCA_FadingOut)
	{
		// Fade IN fast
		CA_CurrentIntensity = FMath::Min(
			CA_CurrentIntensity + HitFX_FadeInSpeed * DeltaSeconds,
			HitFX_Intensity);

		// Count down hold time
		if (CA_CurrentIntensity >= HitFX_Intensity)
		{
			CA_HoldTimeRemaining -= DeltaSeconds;
			if (CA_HoldTimeRemaining <= 0.f)
				bCA_FadingOut = true;
		}
	}
	else
	{
		// Fade OUT slow
		const float FadeSpeed = (HitFX_FadeOutDuration > 0.f)
			? (HitFX_Intensity / HitFX_FadeOutDuration)
			: 1.f;
		CA_CurrentIntensity = FMath::Max(
			CA_CurrentIntensity - FadeSpeed * DeltaSeconds, 0.f);
	}

	// Apply to camera post process
	FollowCamera->PostProcessSettings.bOverride_SceneFringeIntensity = true;
	FollowCamera->PostProcessSettings.SceneFringeIntensity = CA_CurrentIntensity;
	FollowCamera->PostProcessSettings.bOverride_ChromaticAberrationStartOffset = true;
	FollowCamera->PostProcessSettings.ChromaticAberrationStartOffset = 0.f;
}

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
    TriggerChromaticAberration();
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

  // Save run result to run_history
  if (UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr)
  {
    const float Duration = GetWorld() ? (GetWorld()->GetTimeSeconds() - RunStartTime) : 0.f;
    int32 ClearedRooms = 0;
    if (URoomGeneratorSubsystem* RoomGen = GetWorld()->GetSubsystem<URoomGeneratorSubsystem>())
    {
      ClearedRooms = RoomGen->GetClearedRoomsCount();
    }
    Save->SaveRunResult(ClearedRooms, FMath::RoundToInt(Duration), false);
  }

  // Commit 50% run diamonds to profile
  if (PlayerEconomy)
  {
    PlayerEconomy->OnPlayerDeath();
  }

  if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
  {
    MoveComp->StopMovementImmediately();
    MoveComp->DisableMovement();
  }
  ConsumeMovementInputVector();
  SetActorEnableCollision(false);
  bIsMoving = false;

  // Disable all player input — prevent moving/attacking/dashing while dead
  if (APlayerController* PC = Cast<APlayerController>(GetController()))
  {
    DisableInput(PC);
    if (UEnhancedInputLocalPlayerSubsystem* EIS =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer()))
    {
      EIS->ClearAllMappings();
    }
    UE_LOG(LogDepthrun, Log, TEXT("[Player] Input disabled after death"));
  }

  if (FB_Death && GetSprite()) {
    GetSprite()->SetFlipbook(FB_Death);
    GetSprite()->SetLooping(false);
    GetSprite()->Play();
  }
}

float ADepthrunCharacter::Heal(float Amount) {
  if (Amount <= 0.0f || bIsDead || CurrentHP >= MaxHP) {
    return 0.0f;
  }
  float OldHP = CurrentHP;
  CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);
  float ActualHealed = CurrentHP - OldHP;
  UE_LOG(LogDepthrun, Log, TEXT("[Player] Healed: %.0f → %.0f (+%.0f)"), OldHP, CurrentHP, ActualHealed);
  return ActualHealed;
}

void ADepthrunCharacter::ApplyProfileUpgrades()
{
	UDepthrunSaveSubsystem* Save = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>()
		: nullptr;

	if (!Save)
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[Player] ApplyProfileUpgrades: no SaveSubsystem"));
		return;
	}

	// Load upgrade levels
	int32 DamageLvl     = Save->GetUpgradeLevel(EHubUpgrade::Damage);
	int32 RangeLvl      = Save->GetUpgradeLevel(EHubUpgrade::Range);
	int32 ArrowCountLvl = Save->GetUpgradeLevel(EHubUpgrade::ArrowCount);
	int32 MaxHPLvl      = Save->GetUpgradeLevel(EHubUpgrade::MaxHP);

	// Apply multipliers
	DamageMultiplier       = HubUpgradeConfig::GetDamageMultiplier(DamageLvl);
	MeleeRangeMultiplier   = HubUpgradeConfig::GetMeleeRangeMultiplier(RangeLvl);
	BaseProjectileCount    = HubUpgradeConfig::GetBaseProjectileCount(ArrowCountLvl);

	// Apply MaxHP bonus (base 500 + bonus)
	float HPBonus = HubUpgradeConfig::GetMaxHPBonus(MaxHPLvl);
	MaxHP = 500.f + HPBonus;
	CurrentHP = FMath::Min(CurrentHP, MaxHP); // Clamp current HP to new max

	UE_LOG(LogDepthrunSave, Log, TEXT("[Player] Profile upgrades applied: Damage x%.2f, Range x%.2f, Arrows %d, MaxHP %.0f"),
		DamageMultiplier, MeleeRangeMultiplier, BaseProjectileCount, MaxHP);
}

// ─── Console Commands Implementation ──────────────────────────────────────────

void ADepthrunCharacter::AddRunDiamonds(int32 Amount)
{
	if (PlayerEconomy && Amount > 0)
	{
		PlayerEconomy->AddDiamonds(Amount);
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Added %d run diamonds"), Amount);
	}
}

void ADepthrunCharacter::AddProfileDiamonds(int32 Amount)
{
	if (UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr)
	{
		Save->AddDiamondsToProfile(Amount);
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Added %d profile diamonds"), Amount);
	}
	else
	{
		UE_LOG(LogDepthrunEconomy, Error, TEXT("[Console] SaveSubsystem not available"));
	}
}

void ADepthrunCharacter::BuyUpgradeCmd(const FString& UpgradeType)
{
	if (UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr)
	{
		EHubUpgrade Type = EHubUpgrade::Damage;
		if (UpgradeType.Equals(TEXT("Damage"), ESearchCase::IgnoreCase)) Type = EHubUpgrade::Damage;
		else if (UpgradeType.Equals(TEXT("Range"), ESearchCase::IgnoreCase)) Type = EHubUpgrade::Range;
		else if (UpgradeType.Equals(TEXT("ArrowCount"), ESearchCase::IgnoreCase) || UpgradeType.Equals(TEXT("Arrows"), ESearchCase::IgnoreCase)) Type = EHubUpgrade::ArrowCount;
		else if (UpgradeType.Equals(TEXT("MaxHP"), ESearchCase::IgnoreCase) || UpgradeType.Equals(TEXT("HP"), ESearchCase::IgnoreCase)) Type = EHubUpgrade::MaxHP;
		else
		{
			UE_LOG(LogDepthrunEconomy, Error, TEXT("[Console] Unknown upgrade type: %s. Use: Damage, Range, ArrowCount, MaxHP"), *UpgradeType);
			return;
		}

		if (Save->BuyUpgrade(Type))
		{
			UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Successfully bought %s upgrade"), *UpgradeType);
			ApplyProfileUpgrades(); // Re-apply immediately
		}
		else
		{
			UE_LOG(LogDepthrunEconomy, Warning, TEXT("[Console] Failed to buy %s upgrade (max level or insufficient diamonds)"), *UpgradeType);
		}
	}
	else
	{
		UE_LOG(LogDepthrunEconomy, Error, TEXT("[Console] SaveSubsystem not available"));
	}
}

void ADepthrunCharacter::ShowProfile()
{
	if (UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr)
	{
		int32 Diamonds = Save->GetTotalDiamonds();
		int32 DmgLvl = Save->GetUpgradeLevel(EHubUpgrade::Damage);
		int32 RangeLvl = Save->GetUpgradeLevel(EHubUpgrade::Range);
		int32 ArrowLvl = Save->GetUpgradeLevel(EHubUpgrade::ArrowCount);
		int32 MaxHPLvl = Save->GetUpgradeLevel(EHubUpgrade::MaxHP);

		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] ===== PROFILE ====="));
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Total Diamonds: %d"), Diamonds);
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Damage Lvl: %d (Cost: %d)"), DmgLvl, Save->GetUpgradeCost(EHubUpgrade::Damage));
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Range Lvl: %d (Cost: %d)"), RangeLvl, Save->GetUpgradeCost(EHubUpgrade::Range));
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] ArrowCount Lvl: %d (Cost: %d)"), ArrowLvl, Save->GetUpgradeCost(EHubUpgrade::ArrowCount));
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] MaxHP Lvl: %d (Cost: %d)"), MaxHPLvl, Save->GetUpgradeCost(EHubUpgrade::MaxHP));
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] ==================="));
	}
	else
	{
		UE_LOG(LogDepthrunEconomy, Error, TEXT("[Console] SaveSubsystem not available"));
	}
}

void ADepthrunCharacter::ResetProfileCmd()
{
	if (UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr)
	{
		Save->ResetProfile();
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Console] Profile reset to defaults"));
		ApplyProfileUpgrades();
	}
	else
	{
		UE_LOG(LogDepthrunEconomy, Error, TEXT("[Console] SaveSubsystem not available"));
	}
}

void ADepthrunCharacter::GiveItem(const FString& ItemName)
{
	if (!ItemCollection)
	{
		UE_LOG(LogDepthrun, Error, TEXT("[Console] GiveItem: ItemCollection not assigned in BP_DepthrunCharacter!"));
		return;
	}
	if (!ItemInventory)
	{
		UE_LOG(LogDepthrun, Error, TEXT("[Console] GiveItem: ItemInventory component missing"));
		return;
	}

	const FRunItemData* Found = ItemCollection->FindByName(ItemName);
	if (!Found)
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[Console] GiveItem: '%s' not found. Use ListItems to see available items."), *ItemName);
		return;
	}

	const bool bAdded = ItemInventory->AddItem(*Found);
	if (bAdded)
	{
		if (SpawnedWeapon1) ItemInventory->ApplyToWeapon(SpawnedWeapon1);
		if (SpawnedWeapon2) ItemInventory->ApplyToWeapon(SpawnedWeapon2);
		ItemInventory->ApplyToCharacter(this);
		UE_LOG(LogDepthrun, Log, TEXT("[Console] GiveItem: gave '%s', applied"), *Found->ItemName);
	}
	else
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[Console] GiveItem: could not add '%s' (inventory full)"), *Found->ItemName);
	}
}

void ADepthrunCharacter::ListItems()
{
	if (!ItemCollection)
	{
		UE_LOG(LogDepthrun, Error, TEXT("[Console] ListItems: ItemCollection not assigned in BP_DepthrunCharacter!"));
		return;
	}
	UE_LOG(LogDepthrun, Log, TEXT("[Console] === Available Items (%d) ==="), ItemCollection->Items.Num());
	for (int32 i = 0; i < ItemCollection->Items.Num(); ++i)
	{
		const FRunItemData& Item = ItemCollection->Items[i];
		UE_LOG(LogDepthrun, Log, TEXT("[Console]  [%d] %s — %s"),
			i, *Item.ItemName, *UEnum::GetValueAsString(Item.Effect));
	}
	UE_LOG(LogDepthrun, Log, TEXT("[Console] Usage: GiveItem <name>"));
}

void ADepthrunCharacter::ClearRunItems()
{
	if (!ItemInventory)
	{
		UE_LOG(LogDepthrun, Error, TEXT("[Console] ClearRunItems: ItemInventory missing"));
		return;
	}
	ItemInventory->ClearItems();
	// Re-apply profile upgrades so base stats are restored
	ApplyProfileUpgrades();
	if (SpawnedWeapon1) ItemInventory->ApplyToWeapon(SpawnedWeapon1);
	if (SpawnedWeapon2) ItemInventory->ApplyToWeapon(SpawnedWeapon2);
	UE_LOG(LogDepthrun, Log, TEXT("[Console] ClearRunItems: all items cleared, base stats restored"));
}

void ADepthrunCharacter::DBCheck()
{
	UDepthrunSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>() : nullptr;
	USQLiteManager* DB = Save ? Save->GetDB() : nullptr;
	if (!DB)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[Console] DBCheck: DB not open"));
		return;
	}

	// player_profile
	const TArray<TMap<FString,FString>> Profile = DB->SelectRows(TEXT("player_profile"), TEXT("id=1"));
	if (Profile.Num() > 0)
	{
		const TMap<FString,FString>& Row = Profile[0];
		UE_LOG(LogDepthrunSave, Log, TEXT("[DBCheck] player_profile: Diamonds=%s Dmg=%s Range=%s Arrows=%s HP=%s"),
			Row.Contains(TEXT("TotalDiamonds")) ? *Row[TEXT("TotalDiamonds")] : TEXT("?"),
			Row.Contains(TEXT("Damage_Lvl"))    ? *Row[TEXT("Damage_Lvl")]    : TEXT("?"),
			Row.Contains(TEXT("Range_Lvl"))     ? *Row[TEXT("Range_Lvl")]     : TEXT("?"),
			Row.Contains(TEXT("ArrowCount_Lvl"))? *Row[TEXT("ArrowCount_Lvl")]: TEXT("?"),
			Row.Contains(TEXT("MaxHP_Lvl"))     ? *Row[TEXT("MaxHP_Lvl")]     : TEXT("?"));
	}
	else
	{
		UE_LOG(LogDepthrunSave, Warning, TEXT("[DBCheck] player_profile: no rows"));
	}

	// run_history
	const TArray<TMap<FString,FString>> Runs = DB->SelectRows(TEXT("run_history"), TEXT("1=1"));
	UE_LOG(LogDepthrunSave, Log, TEXT("[DBCheck] run_history: %d rows"), Runs.Num());
	for (const TMap<FString,FString>& Row : Runs)
	{
		UE_LOG(LogDepthrunSave, Log, TEXT("[DBCheck]  id=%s Rooms=%s Won=%s Duration=%ss Timestamp=%s"),
			Row.Contains(TEXT("id"))          ? *Row[TEXT("id")]          : TEXT("?"),
			Row.Contains(TEXT("Rooms"))       ? *Row[TEXT("Rooms")]       : TEXT("?"),
			Row.Contains(TEXT("Won"))         ? *Row[TEXT("Won")]         : TEXT("?"),
			Row.Contains(TEXT("RunDuration")) ? *Row[TEXT("RunDuration")] : TEXT("?"),
			Row.Contains(TEXT("Timestamp"))   ? *Row[TEXT("Timestamp")]   : TEXT("?"));
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

  if (!Desired) {
    switch (FacingDirection) {
    case EPlayerFacingDirection::Right:
    case EPlayerFacingDirection::Left:
      Desired = FB_WalkRight ? FB_WalkRight : FB_IdleRight;
      break;
    case EPlayerFacingDirection::Up:
      Desired = FB_WalkUp ? FB_WalkUp : FB_IdleUp;
      break;
    case EPlayerFacingDirection::Down:
      Desired = FB_WalkDown ? FB_WalkDown : FB_IdleDown;
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
