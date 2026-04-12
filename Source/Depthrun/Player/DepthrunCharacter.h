// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "InputActionValue.h"
#include "Combat/BaseWeapon.h"       // EWeaponType (needed for attack tracking)
#include "DepthrunCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UPaperFlipbook;
class UInputMappingContext;
class UInputAction;
class UPlayerCombatComponent;
class UPlayerActionTracker;

/** Which direction the character sprite faces. */
UENUM(BlueprintType)
enum class EPlayerFacingDirection : uint8
{
	Right UMETA(DisplayName = "Right"),
	Left  UMETA(DisplayName = "Left"),
	Up    UMETA(DisplayName = "Up"),
	Down  UMETA(DisplayName = "Down")
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
class DEPTHRUN_API ADepthrunCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ADepthrunCharacter();

	UPlayerActionTracker* GetActionTracker() const { return ActionTracker; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Input handlers ---
	void HandleMove(const FInputActionValue& Value);
	void HandleAttack(const FInputActionValue& Value);
	void HandleDash(const FInputActionValue& Value);

	// --- Internal ---
	void Dash();
	void OnDashCooldownEnd();
	void UpdateFacingDirection(const FVector2D& Input);
	void UpdateAnimation();
	FVector GetFireDirection() const;

public:
	// ────────────────────── Components ──────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UPlayerCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UPlayerActionTracker> ActionTracker;

	// ────────────────────── Stats ────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stats")
	float MaxHP = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Stats")
	float CurrentHP = 100.f;

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

	// ────────────────────── Combat ───────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Combat")
	TSubclassOf<ABaseWeapon> DefaultWeaponClass;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Melee")
	TObjectPtr<UPaperFlipbook> FB_MeleeAttackRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Melee")
	TObjectPtr<UPaperFlipbook> FB_MeleeAttackUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Melee")
	TObjectPtr<UPaperFlipbook> FB_MeleeAttackDown;

	// --- RANGED ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Ranged")
	TObjectPtr<UPaperFlipbook> FB_RangedAttackRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Ranged")
	TObjectPtr<UPaperFlipbook> FB_RangedAttackUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Attack|Ranged")
	TObjectPtr<UPaperFlipbook> FB_RangedAttackDown;

	// ────────────────────── Dash ─────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Dash")
	float DashImpulse = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Dash")
	float DashCooldown = 1.0f;

private:
	bool bCanDash    = true;
	bool bIsMoving   = false;
	bool bIsAttacking= false;
	FTimerHandle DashCooldownTimer;
	/** Stops post-dash sliding after DashStopDelay seconds. */
	FTimerHandle DashStopTimer;
	/** Resets bIsAttacking after the weapon cooldown expires. */
	FTimerHandle AttackAnimTimer;
};
