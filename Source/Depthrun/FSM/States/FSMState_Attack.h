// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMState.h"
#include "FSMState_Attack.generated.h"

/**
 * UFSMState_Attack
 * Enemy performs an attack action (melee close / ranged shoot).
 * Respects per-enemy cooldown. Logic implemented in Stage 5B.
 */
UCLASS()
class DEPTHRUN_API UFSMState_Attack : public UFSMState
{
	GENERATED_BODY()

public:
	virtual void EnterState(ABaseEnemy* Owner) override;
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime) override;
	virtual void ExitState(ABaseEnemy* Owner) override;

	virtual EFSMStateType GetStateType() const override
	{
		return EFSMStateType::Attack;
	}

	/** Time between attack actions (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "FSM|Attack")
	float AttackCooldown = 1.2f;

private:
	float TimeSinceLastAttack = 0.f;
	bool  bAttackCoolingDown  = false;
};
