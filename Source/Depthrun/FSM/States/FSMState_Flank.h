// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMState.h"
#include "FSMState_Flank.generated.h"

/**
 * UFSMState_Flank
 * Enemy moves perpendicular to the player-to-enemy vector,
 * attempting to reach the player's side or rear.
 * Logic implemented in Stage 5B.
 */
UCLASS()
class DEPTHRUN_API UFSMState_Flank : public UFSMState
{
	GENERATED_BODY()

public:
	virtual void EnterState(ABaseEnemy* Owner) override;
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime) override;
	virtual void ExitState(ABaseEnemy* Owner) override;

	virtual EFSMStateType GetStateType() const override
	{
		return EFSMStateType::Flank;
	}

	/** How long (seconds) to maintain flank direction before recalculating. */
	UPROPERTY(EditDefaultsOnly, Category = "FSM|Flank")
	float FlankDirectionRefreshInterval = 1.5f;

private:
	/** Current perpendicular direction to strafe in. */
	FVector FlankDirection = FVector::ZeroVector;
	float   TimeSinceDirectionRefresh = 0.f;
	/** +1 or -1: left or right of player. Randomised on Enter. */
	int32   FlankSide = 1;
};
