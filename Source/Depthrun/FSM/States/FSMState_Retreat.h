// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMState.h"
#include "FSMState_Retreat.generated.h"

/**
 * UFSMState_Retreat
 * Enemy moves away from the player in a safe direction (not into walls).
 * Logic implemented in Stage 5B.
 */
UCLASS()
class DEPTHRUN_API UFSMState_Retreat : public UFSMState
{
	GENERATED_BODY()

public:
	virtual void EnterState(ABaseEnemy* Owner) override;
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime) override;
	virtual void ExitState(ABaseEnemy* Owner) override;

	virtual EFSMStateType GetStateType() const override
	{
		return EFSMStateType::Retreat;
	}

	/** Minimum safe distance to maintain from player (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "FSM|Retreat")
	float SafeDistance = 600.f;

private:
	/** Cached retreat direction (recalculated on Enter and periodically). */
	FVector RetreatDirection = FVector::ZeroVector;
};
