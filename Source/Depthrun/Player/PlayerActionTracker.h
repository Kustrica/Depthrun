// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerActionTracker.generated.h"

/**
 * EPlayerActionType
 * Defined here (Stage 3A); will be referenced by AdaptiveBehavior module (Stage 6A).
 */
UENUM(BlueprintType)
enum class EPlayerActionType : uint8
{
	Shot           UMETA(DisplayName = "Shot"),
	Dash           UMETA(DisplayName = "Dash"),
	MeleeAttack    UMETA(DisplayName = "Melee Attack"),
	Heal           UMETA(DisplayName = "Heal"),
	SpecialAbility UMETA(DisplayName = "Special Ability")
};

/** Carries full context of one player action for the enemy memory system. */
USTRUCT(BlueprintType)
struct FPlayerActionEvent
{
	GENERATED_BODY()

	UPROPERTY() EPlayerActionType ActionType = EPlayerActionType::Shot;
	UPROPERTY() float             Timestamp  = 0.f;
	UPROPERTY() FVector           Location   = FVector::ZeroVector;
	/** Intensity [0,1] — reserved for future use by time-decay memory. */
	UPROPERTY() float             Intensity  = 1.f;
};

/** Broadcast whenever the player performs a trackable action. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAction, const FPlayerActionEvent&, ActionEvent);

/**
 * UPlayerActionTracker
 * Records every player action with timestamp/location and broadcasts it.
 * Enemy AdaptiveBehaviorComponent subscribes to OnPlayerAction to feed
 * the time-decay memory and N-gram pattern recognizer.
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API UPlayerActionTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerActionTracker();

	/** Call whenever the player performs an action. Broadcasts the event. */
	void RecordAction(EPlayerActionType ActionType, const FVector& Location, float Intensity = 1.f);

	/** All enemies subscribe here to receive player action events. */
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnPlayerAction OnPlayerAction;

private:
	float GetCurrentTimestamp() const;
};
