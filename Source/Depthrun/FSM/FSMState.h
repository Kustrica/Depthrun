// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FSM/FSMTypes.h"
#include "FSMState.generated.h"

class ABaseEnemy;

/**
 * UFSMState
 * Abstract base for all five FSM states (Idle, Chase, Attack, Retreat, Flank).
 *
 * Rules (from architecture):
 *  - States contain movement/action LOGIC only — no decision logic.
 *  - Decision (which state to enter next) is made by UAdaptiveBehaviorComponent.
 *  - FSMComponent calls Enter/Tick/Exit; it does NOT decide transitions.
 *
 * Lifecycle:
 *  BeginPlay → UFSMComponent::RegisterState → AEnemy runs tick loop
 *  → UFSMComponent::TransitionTo → ExitState(old) → EnterState(new)
 */
UCLASS(Abstract)
class DEPTHRUN_API UFSMState : public UObject
{
	GENERATED_BODY()

public:
	/** Called once when FSM enters this state. Reset timers/state here. */
	virtual void EnterState(ABaseEnemy* Owner);

	/** Called every frame while this state is active. Implement movement/attack here. */
	virtual void TickState(ABaseEnemy* Owner, float DeltaTime);

	/** Called once when FSM leaves this state. Clean up anything started in Enter. */
	virtual void ExitState(ABaseEnemy* Owner);

	/** Returns the enum type of this state. Must be overridden by each concrete state. */
	virtual EFSMStateType GetStateType() const
		PURE_VIRTUAL(UFSMState::GetStateType, return EFSMStateType::None;);

	/** Returns time (seconds) spent continuously in this state. Reset on Enter. */
	float GetTimeInState() const { return TimeInState; }

protected:
	/** Accumulated time since last EnterState(). Updated by TickState base call. */
	float TimeInState = 0.f;
};
