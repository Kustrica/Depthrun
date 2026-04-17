// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMState.h"
#include "FSMState_Idle.generated.h"

/**
 * UFSMState_Idle
 * Enemy stands still. Requests re-evaluation every N seconds.
 * Transitions: → Chase (player detected), → Attack (player very close).
 * Logic implemented in Stage 5B.
 */
UCLASS()
class DEPTHRUN_API UFSMState_Idle : public UFSMState
{
	GENERATED_BODY()

public:
	virtual void EnterState(ABaseEnemy* Owner) override;
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime) override;
	virtual void ExitState(ABaseEnemy* Owner) override;

	virtual EFSMStateType GetStateType() const override
	{
		return EFSMStateType::Idle;
	}

private:
	/** Time since last "look around" animation cue. */
	float TimeSinceLastLook = 0.f;
};
