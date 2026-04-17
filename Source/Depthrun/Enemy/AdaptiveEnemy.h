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
class DEPTHRUN_API AAdaptiveEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AAdaptiveEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void OnKilled() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFSMComponent> FSMComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAdaptiveBehaviorComponent> AdaptiveComp;
};
