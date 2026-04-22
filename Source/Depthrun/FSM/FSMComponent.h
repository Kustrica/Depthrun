// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FSM/FSMTypes.h"
#include "FSMComponent.generated.h"

class UFSMState;
class ABaseEnemy;

/**
 * UFSMComponent
 * Manages the active FSM state for its owner enemy actor.
 *
 * Architecture rules:
 *  - Does NOT make decisions (no threat, no utility scoring here).
 *  - UAdaptiveBehaviorComponent calls TransitionTo() to drive the FSM.
 *  - Simple enemies may drive it with hardcoded timers/distance checks.
 *  - Compiles independently from the AdaptiveBehavior module.
 *
 * Usage:
 *  1. RegisterState(EFSMStateType::Idle,  NewObject<UFSMState_Idle>(this));
 *  2. TransitionTo(EFSMStateType::Idle);   // start in Idle
 *  3. TickComponent ticks CurrentState automatically.
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API UFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFSMComponent();

	// ─── Lifecycle ────────────────────────────────────────────────────────────

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// ─── State Registry ───────────────────────────────────────────────────────

	/** Register a state instance. Call once per state in BeginPlay of the enemy. */
	UFUNCTION(BlueprintCallable, Category = "FSM")
	void RegisterState(EFSMStateType StateType, UFSMState* StateObject);

	// ─── Transitions ─────────────────────────────────────────────────────────

	/**
	 * Transition to a new state: calls ExitState on old, EnterState on new.
	 * No-op if NewState == CurrentStateType and no force is requested.
	 */
	UFUNCTION(BlueprintCallable, Category = "FSM")
	void TransitionTo(EFSMStateType NewState);

	// ─── Queries ─────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "FSM")
	EFSMStateType GetCurrentStateType() const { return CurrentStateType; }

	UFUNCTION(BlueprintPure, Category = "FSM")
	float GetTimeInCurrentState() const;

	static FString GetStateName(EFSMStateType State);

	// ─── Events ──────────────────────────────────────────────────────────────

	/** Broadcast whenever a state transition occurs. Subscribed by debug widget and logger. */
	UPROPERTY(BlueprintAssignable, Category = "FSM|Events")
	FOnFSMStateChanged OnStateChanged;

private:
	/** Registered state objects keyed by their type. */
	UPROPERTY()
	TMap<EFSMStateType, TObjectPtr<UFSMState>> States;

	/** Currently active state object. Null before first TransitionTo(). */
	UPROPERTY()
	TObjectPtr<UFSMState> CurrentState;

	EFSMStateType CurrentStateType = EFSMStateType::None;

	/** Cached owner cast — set in BeginPlay for state tick calls. */
	UPROPERTY()
	TObjectPtr<ABaseEnemy> OwnerEnemy;
};
