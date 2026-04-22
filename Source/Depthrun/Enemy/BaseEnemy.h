#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "BaseEnemy.generated.h"

class UEnemyHealthComponent;
class UFSMComponent;
class UPaperFlipbook;

UENUM(BlueprintType)
enum class EEnemyType : uint8 {
  Melee UMETA(DisplayName = "Melee"),
  Ranged UMETA(DisplayName = "Ranged"),
  Adaptive UMETA(DisplayName = "Adaptive")
};

UCLASS(Abstract)
class DEPTHRUN_API ABaseEnemy : public APaperCharacter {
  GENERATED_BODY()

public:
  ABaseEnemy();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UEnemyHealthComponent> HealthComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UFSMComponent> FSMComponent;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
  EEnemyType EnemyType;

  UFUNCTION()
  virtual void OnDeath();

  void UpdateAnimation();

public:
  EEnemyType GetEnemyType() const { return EnemyType; }

  // ── Config ─────────────────────────────────────────────────────────────

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<UPaperFlipbook> FB_Idle;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<UPaperFlipbook> FB_Walk;

  /** Plays when the enemy is in Attack FSM state. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<UPaperFlipbook> FB_Attack;

  /** Plays briefly when taking damage. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<UPaperFlipbook> FB_Hit;

  /** Plays on death. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<UPaperFlipbook> FB_Death;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Stats")
  float MoveSpeed = 300.f;

  /** Range at which the enemy first detects the player and starts chasing. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float DetectionRange = 500.f;

  /** Range at which the enemy enters Attack state. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float AttackRange = 60.f;

  /** Ranged enemies will try to retreat if player is closer than this. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float MinAttackRange = 200.f;

  /** Distance at which the enemy stops retreating. Default 350 is enough to
   * clear melee range. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float SafeDistance = 350.f;

  /** Damage dealt per melee attack. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float AttackDamage = 10.f;

  /** Seconds between melee attacks. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float AttackCooldown = 1.5f;

  /** Optional: weapon class to spawn for this enemy. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  TSubclassOf<class ABaseWeapon> WeaponClass;

  /** For Ranged/Adaptive types: class of projectile to spawn (if not using
   * WeaponClass). */
  UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
  TSubclassOf<class ABaseProjectile> ProjectileClass;

  /** Projectile speed in UU/s. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float ProjectileSpeed = 800.f;

  /** Delay between clicking and actual arrow spawn (to match animation). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float ShotDelay = 0.15f;

  /** For Ranged/Adaptive types: attacks per second. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float FireRate = 1.0f;

  /**
   * Called by FSMState_Attack to trigger a melee attack on the player.
   * Default implementation applies damage via TakeDamage to the nearest player.
   */
  virtual void PerformMeleeAttack();

  virtual float TakeDamage(float DamageAmount,
                           struct FDamageEvent const &DamageEvent,
                           class AController *EventInstigator,
                           AActor *DamageCauser) override;

  virtual void OnSpawned();
  virtual void OnKilled();

protected:
  bool bIsHitAnimationActive = false;
  bool bIsDead = false;
  FTimerHandle HitAnimationTimer;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  TObjectPtr<class ABaseWeapon> SpawnedWeapon;

  void ResetHitAnimation();
};
