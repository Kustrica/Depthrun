// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "AdaptiveEnemy.generated.h"

class UFSMComponent;
class UAdaptiveBehaviorComponent;

/**
 * AAdaptiveEnemy
 * Main demonstration enemy for the diploma.
 * Owns UFSMComponent + UAdaptiveBehaviorComponent.
 *
 * Architecture rule: contains NO behavior logic.
 * All decisions are made by UAdaptiveBehaviorComponent.
 * All state execution is done by UFSMState_* objects via UFSMComponent.
 *
 * BeginPlay (Stage 7):
 *   1. Register 5 FSM states
 *   2. Transition to Idle
 *   3. Subscribe to PlayerActionTracker::OnPlayerAction
 *   4. Subscribe to HealthComponent damage delegates for reward signals
 */
UCLASS()
class DEPTHRUN_API AAdaptiveEnemy : public ABaseEnemy {
  GENERATED_BODY()

public:
  AAdaptiveEnemy();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
  bool bIsRangedMode = false;

  virtual void PerformMeleeAttack() override;

  /** For ranged mode: fires a projectile. */
  void ActuallyFire();

protected:
  virtual void BeginPlay() override;
  virtual void OnKilled() override;

  UFUNCTION()
  void HandleHealthChanged(float OldHP, float NewHP, float MaxHP);

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  TObjectPtr<UAdaptiveBehaviorComponent> AdaptiveComp;

  // ─── Animation: Ranged Mode ─────────────────────────────────────────────

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<class UPaperFlipbook> FB_Idle_Ranged;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<class UPaperFlipbook> FB_Walk_Ranged;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<class UPaperFlipbook> FB_Attack_Ranged;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<class UPaperFlipbook> FB_Hit_Ranged;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
  TObjectPtr<class UPaperFlipbook> FB_Death_Ranged;


  virtual void UpdateAnimation() override;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
  float MuzzleOffset = 40.f;
};
