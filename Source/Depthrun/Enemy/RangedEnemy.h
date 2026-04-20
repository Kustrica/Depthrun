// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "RangedEnemy.generated.h"

class ABaseProjectile;

/**
 * ARangedEnemy
 * Ranged enemy subclass. Overrides PerformMeleeAttack() to fire a projectile
 * toward the player in full 360° (no axis snapping). Configured via Blueprint:
 * assign ProjectileClass and tune AttackRange/AttackCooldown in Details panel.
 */
UCLASS()
class DEPTHRUN_API ARangedEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	ARangedEnemy();

	// ─── Ranged Combat Config ─────────────────────────────────────────────

	/** Spawn offset (UU) in the fire direction so the projectile clears the capsule. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Ranged")
	float MuzzleOffset = 40.f;

	/**
	 * Fires a projectile toward the player in any 360° direction.
	 * Called by FSMState_Attack on cooldown. Named "PerformMeleeAttack" to match
	 * the base class virtual — FSM calls the same method for all enemy types.
	 */
	virtual void PerformMeleeAttack() override;

private:
	void ActuallyFire();
};
