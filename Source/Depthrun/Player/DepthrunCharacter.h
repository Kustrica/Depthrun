// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "Combat/BaseWeapon.h" // EWeaponType (needed for attack tracking)
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "PaperCharacter.h"
#include "DepthrunCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPaperFlipbook;
class UInputMappingContext;
class UInputAction;
class UPlayerCombatComponent;
class UPlayerActionTracker;
class URunItemInventory;
class URunItemCollection;
class UPlayerMovementConfig;
class UCombatMusicTrigger;
class UPlayerEconomy;

/** Which direction the character sprite faces. */
UENUM(BlueprintType)
enum class EPlayerFacingDirection : uint8 {
  Right UMETA(DisplayName = "Right"),
  Left UMETA(DisplayName = "Left"),
  Up UMETA(DisplayName = "Up"),
  Down UMETA(DisplayName = "Down")
};

/**
 * ADepthrunCharacter
 * Top-down 2D Paper2D character. Drives movement via Enhanced Input.
 * Owns UPlayerCombatComponent and UPlayerActionTracker.
 * Contains NO behavior logic — pure input-to-movement/action forwarding.
 *
 * Blueprint subclass (BP_DepthrunCharacter) should assign:
 *   - DefaultMappingContext, IA_Move, IA_Attack, IA_Dash
 *   - All FB_* flipbook assets
 */
UCLASS()
class DEPTHRUN_API ADepthrunCharacter : public APaperCharacter {
  GENERATED_BODY()

public:
  ADepthrunCharacter();

  UPlayerActionTracker *GetActionTracker() const { return ActionTracker; }
  bool IsDead() const { return bIsDead; }

protected:
  virtual void BeginPlay() override;
  virtual void
  SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

  // --- Input handlers ---
  void HandleMove(const FInputActionValue &Value);
  void HandleAttack(const FInputActionValue &Value);
  void HandleDash(const FInputActionValue &Value);
  void HandleSwitchSlot1(const FInputActionValue &Value);
  void HandleSwitchSlot2(const FInputActionValue &Value);
  void HandleUsePotion(const FInputActionValue &Value);

  // --- Internal ---
  void Dash();
  void OnDashCooldownEnd();
  void UpdateFacingDirection(const FVector2D &Input);
  void UpdateAnimation();
  FVector GetFireDirection() const;
  
  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
  void Die();

public:
  /** Heal the player, clamped to MaxHP. Returns actual amount healed. */
  UFUNCTION(BlueprintCallable, Category = "Player|Stats")
  float Heal(float Amount);

  /** Load profile upgrades from SaveSubsystem and apply to character stats. */
  UFUNCTION(BlueprintCallable, Category = "Player|Meta")
  void ApplyProfileUpgrades();

  // ─── Console Commands (Debug) ────────────────────────────────────────────

  /** Console: Add diamonds to run (not profile). Usage: AddRunDiamonds 100 */
  UFUNCTION(Exec, Category = "Player|Debug")
  void AddRunDiamonds(int32 Amount);

  /** Console: Add diamonds to profile. Usage: AddProfileDiamonds 1000 */
  UFUNCTION(Exec, Category = "Player|Debug")
  void AddProfileDiamonds(int32 Amount);

  /** Console: Buy upgrade. Usage: BuyUpgrade Damage */
  UFUNCTION(Exec, Category = "Player|Debug")
  void BuyUpgradeCmd(const FString& UpgradeType);

  /** Console: Show current profile. Usage: ShowProfile */
  UFUNCTION(Exec, Category = "Player|Debug")
  void ShowProfile();

  /** Console: Reset profile. Usage: ResetProfile */
  UFUNCTION(Exec, Category = "Player|Debug")
  void ResetProfileCmd();

  /** Console: Give item by name from ItemCollection. Usage: GiveItem ArrowRicochet */
  UFUNCTION(Exec, Category = "Player|Debug")
  void GiveItem(const FString& ItemName);

  /** Console: List all available items in ItemCollection. Usage: ListItems */
  UFUNCTION(Exec, Category = "Player|Debug")
  void ListItems();

  /** Console: Clear all run items. Usage: ClearRunItems */
  UFUNCTION(Exec, Category = "Player|Debug")
  void ClearRunItems();

  // ────────────────────── Components ──────────────────────
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
  TObjectPtr<USpringArmComponent> SpringArm;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
  TObjectPtr<UCameraComponent> FollowCamera;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
  TObjectPtr<UPlayerCombatComponent> CombatComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
  TObjectPtr<UPlayerActionTracker> ActionTracker;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Items")
  TObjectPtr<URunItemInventory> ItemInventory;

  /** Single DataAsset with all available run items. Assign DA_RunItemCollection in BP. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Items")
  TObjectPtr<URunItemCollection> ItemCollection;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Audio")
  TObjectPtr<UCombatMusicTrigger> CombatMusicTrigger;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Economy")
  TObjectPtr<UPlayerEconomy> PlayerEconomy;

public:
  // ────────────────────── Stats ────────────────────────────
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stats")
  float MaxHP = 500.f;

  UPROPERTY(BlueprintReadOnly, Category = "Player|Stats")
  float CurrentHP = 500.f;

  // ────────────────────── Facing ───────────────────────────
  UPROPERTY(BlueprintReadOnly, Category = "Player")
  EPlayerFacingDirection FacingDirection = EPlayerFacingDirection::Right;

  // ────────────────────── Input ────────────────────────────
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputMappingContext> DefaultMappingContext;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_Move;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_Attack;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_Dash;

  /** Key 1 — switch to Sword (Slot 1). Assign in Blueprint / IMC. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_SwitchSlot1;

  /** Key 2 — switch to Bow (Slot 2). Assign in Blueprint / IMC. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_SwitchSlot2;

  /** Key E — use health potion. Assign in Blueprint / IMC. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
  TObjectPtr<UInputAction> IA_UsePotion;

  /** Manual rotation offset for the sprite component to match top-down view. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Rotation")
  FRotator SpriteRotationOffset = FRotator(-90.f, 0.f, 0.f);

  // ────────────────────── Combat / Weapon Slots ────────────
  /** Slot 1 — Melee (Sword). TSubclassOf assigned in BP_DepthrunCharacter. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Combat")
  TSubclassOf<ABaseWeapon> WeaponSlotClass1;

  /** Slot 2 — Ranged (Bow). TSubclassOf assigned in BP_DepthrunCharacter. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Combat")
  TSubclassOf<ABaseWeapon> WeaponSlotClass2;

  /** Currently active weapon slot (1 or 2). Initialized to 0 to force first
   * equip. */
  UPROPERTY(BlueprintReadOnly, Category = "Player|Combat")
  int32 ActiveWeaponSlot = 0;

  /**
   * Switch active weapon to slot 1 (Sword) or slot 2 (Bow).
   * Hides the old weapon, shows the new one, re-applies item effects.
   */
  UFUNCTION(BlueprintCallable, Category = "Combat")
  void SwitchToWeaponSlot(int32 SlotIndex);

  // ─────────────── Flipbooks (assigned in Blueprint) ───────
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Idle")
  TObjectPtr<UPaperFlipbook> FB_IdleRight;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Idle")
  TObjectPtr<UPaperFlipbook> FB_IdleUp;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Idle")
  TObjectPtr<UPaperFlipbook> FB_IdleDown;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Walk")
  TObjectPtr<UPaperFlipbook> FB_WalkRight;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Walk")
  TObjectPtr<UPaperFlipbook> FB_WalkUp;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Walk")
  TObjectPtr<UPaperFlipbook> FB_WalkDown;

  // --- MELEE ---
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Melee")
  TObjectPtr<UPaperFlipbook> FB_MeleeAttackRight;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Melee")
  TObjectPtr<UPaperFlipbook> FB_MeleeAttackUp;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Melee")
  TObjectPtr<UPaperFlipbook> FB_MeleeAttackDown;

  // --- RANGED ---
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Ranged")
  TObjectPtr<UPaperFlipbook> FB_RangedAttackRight;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Ranged")
  TObjectPtr<UPaperFlipbook> FB_RangedAttackUp;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Animation|Attack|Ranged")
  TObjectPtr<UPaperFlipbook> FB_RangedAttackDown;

  // ────────────────────── Dash ─────────────────────────────
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Dash")
  float DashImpulse = 800.f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Dash")
  float DashCooldown = 1.0f;

  /** Config data asset for movement tuning. Assign in BP_DepthrunCharacter. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Config")
  TObjectPtr<UPlayerMovementConfig> MovementConfig;

  /** Multipliers from meta-upgrades. Applied in ApplyProfileUpgrades(). */
  UPROPERTY(BlueprintReadOnly, Category = "Player|Meta")
  float DamageMultiplier = 1.0f;

  UPROPERTY(BlueprintReadOnly, Category = "Player|Meta")
  float MeleeRangeMultiplier = 1.0f;

  UPROPERTY(BlueprintReadOnly, Category = "Player|Meta")
  int32 BaseProjectileCount = 3;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Combat")
  TObjectPtr<UPaperFlipbook> FB_Hit;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Combat")
  TObjectPtr<UPaperFlipbook> FB_Death;

  // ────────────────────── Debug ────────────────────────────
  UPROPERTY(BlueprintReadWrite, Category = "Player|Debug")
  bool bGodMode = false;

  UPROPERTY(BlueprintReadWrite, Category = "Player|Debug")
  bool bSuperAttack = false;

  UFUNCTION(BlueprintCallable, Exec, Category = "Player|Debug")
  void ToggleGodMode() { bGodMode = !bGodMode; }

  UFUNCTION(BlueprintCallable, Exec, Category = "Player|Debug")
  void ToggleSuperAttack() { bSuperAttack = !bSuperAttack; }

  /** Helper functions for HUD Binding - just select these in the Widget Text Bind dropdown! */
  UFUNCTION(BlueprintPure, Category = "Player|Debug")
  FText GetGodStatusText() const;

  UFUNCTION(BlueprintPure, Category = "Player|Debug")
  FText GetSuperAttackStatusText() const;

  /** Called by RoomGeneratorSubsystem after teleport to set the Z floor plane for this character.
   *  The plane constraint in CharacterMovement prevents further Z changes automatically. */
  UFUNCTION(BlueprintCallable, Category = "Player|Spawn")
  void SetLockedZ(float InZ)
  {
    LockedPlayerZ = InZ;
    FVector Loc = GetActorLocation();
    Loc.Z = InZ;
    SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
    // Update the plane constraint origin so the CMC snaps to the correct height
    if (GetCharacterMovement())
    {
      GetCharacterMovement()->SetPlaneConstraintOrigin(FVector(0.f, 0.f, InZ));
    }
  }

private:
  bool bCanDash = true;
  bool bIsMoving = false;
  bool bIsAttacking = false;
  bool bIsHitAnimationActive = false;
  bool bIsDead = false;

  float LockedPlayerZ = 4.0f;
  bool  bZLocked = false;

  FTimerHandle DashCooldownTimer;
  FTimerHandle DashStopTimer;
  FTimerHandle AttackAnimTimer;
  FTimerHandle HitAnimTimer;

  void ResetHitAnimation() { bIsHitAnimationActive = false; }

  /** Live weapon actor instances (spawned in BeginPlay). */
  UPROPERTY()
  TObjectPtr<ABaseWeapon> SpawnedWeapon1; // Sword
  UPROPERTY()
  TObjectPtr<ABaseWeapon> SpawnedWeapon2; // Bow
};
