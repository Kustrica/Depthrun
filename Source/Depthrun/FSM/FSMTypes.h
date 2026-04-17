// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSMTypes.generated.h"

/**
 * EFSMStateType
 * The five states of the custom FSM used by all enemy types.
 * Defined here so FSM and AdaptiveBehavior modules share the same type
 * without circular dependencies.
 */
UENUM(BlueprintType)
enum class EFSMStateType : uint8
{
	None    UMETA(DisplayName = "None"),
	Idle    UMETA(DisplayName = "Idle"),
	Chase   UMETA(DisplayName = "Chase"),
	Attack  UMETA(DisplayName = "Attack"),
	Retreat UMETA(DisplayName = "Retreat"),
	Flank   UMETA(DisplayName = "Flank")
};

/** Delegate broadcast by UFSMComponent when a state transition occurs. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFSMStateChanged,
	EFSMStateType, OldState,
	EFSMStateType, NewState);
