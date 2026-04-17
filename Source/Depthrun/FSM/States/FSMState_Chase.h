// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMState.h"
#include "FSMState_Chase.generated.h"

/**
 * UFSMState_Chase
 * Enemy moves toward the player. Transitions to Attack when close enough.
 * Logic implemented in Stage 5B.
 */
UCLASS()
class DEPTHRUN_API UFSMState_Chase : public UFSMState
{
	GENERATED_BODY()

public:
	virtual void EnterState(ABaseEnemy* Owner) override;
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime) override;
	virtual void ExitState(ABaseEnemy* Owner) override;

	virtual EFSMStateType GetStateType() const override
	{
		return EFSMStateType::Chase;
	}

	/** Distance threshold (cm) at which Chase requests transition to Attack. */
	UPROPERTY(EditDefaultsOnly, Category = "FSM|Chase")
	float AttackRangeThreshold = 120.f;
};
