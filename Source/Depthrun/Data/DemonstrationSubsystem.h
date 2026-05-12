// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DemonstrationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDemoCombatEvent : uint8
{
	PlayerShot      UMETA(DisplayName = "PlayerShot"),
	PlayerMelee     UMETA(DisplayName = "PlayerMelee"),
	EnemyShot       UMETA(DisplayName = "EnemyShot"),
	EnemyMelee      UMETA(DisplayName = "EnemyMelee"),
	DamageReceived  UMETA(DisplayName = "DamageReceived"),
};

/**
 * UDemonstrationSubsystem
 * Diagnostic logging subsystem for the adaptive behaviour module.
 * Writes evaluation results, weight updates, state transitions and
 * combat events to the LogAdaptiveBehavior console channel.
 *
 * Accessed via GetGameInstance()->GetSubsystem<UDemonstrationSubsystem>()
 */
UCLASS()
class DEPTHRUN_API UDemonstrationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ─── Logging ─────────────────────────────────────────────────────────────

	/**
	 * Log one full evaluation cycle result.
	 * Called by UAdaptiveBehaviorComponent after each evaluation tick.
	 */
	void LogEvaluationResult(float ThreatValue, const TArray<float>& Weights,
		const FString& ChosenState, const FString& Pattern);

	/**
	 * Log a combat event (shot, melee hit, damage received).
	 * Instigator and Target are optional display names for context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Demo|Log")
	void LogCombatEvent(EDemoCombatEvent EventType,
		const FString& Instigator, const FString& Target);

	/**
	 * Log a weight update step.
	 * OldWeights and NewWeights must each contain exactly 6 elements.
	 */
	void LogWeightUpdate(const TArray<float>& OldWeights,
		const TArray<float>& NewWeights, float Reward);

	/**
	 * Log a FSM state transition with the current threat value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Demo|Log")
	void LogStateTransition(const FString& FromState,
		const FString& ToState, float ThreatValue);

	// ─── Settings ─────────────────────────────────────────────────────────────

	/** When true, all six weights, threat value, chosen state and combat events
	 *  are written to the log on every evaluation cycle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	bool bVerboseLogging = false;
};
