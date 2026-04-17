// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FSM/FSMTypes.h"              // EFSMStateType — shared with FSM module
#include "Player/PlayerActionTracker.h" // EPlayerActionType, FPlayerActionEvent
#include "Combat/BaseWeapon.h"          // EWeaponType
#include "AdaptiveTypes.generated.h"

/**
 * FMemoryEvent
 * A single time-stamped player action stored in the enemy's adaptive memory.
 * Intensity decays exponentially over time via M(t) = Intensity * exp(-λ * Δt).
 */
USTRUCT(BlueprintType)
struct FMemoryEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Memory")
	EPlayerActionType ActionType = EPlayerActionType::Shot;

	/** Absolute game time (GetWorld()->GetTimeSeconds()) when the event occurred. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Memory")
	float Timestamp = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Memory")
	FVector Location = FVector::ZeroVector;

	/** Base intensity [0, 1]. Multiplied by the time-decay factor. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Memory")
	float Intensity = 1.f;
};

/**
 * FContextData
 * Layer 1 output: all normalized [0,1] inputs collected by UContextEvaluator.
 * Passed down the pipeline to Layer 2 (ThreatCalculator).
 */
USTRUCT(BlueprintType)
struct FContextData
{
	GENERATED_BODY()

	/** Raw distance in world units (before normalization). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float DistanceToPlayer = 0.f;

	/** Normalized distance D_norm = clamp(1 - dist/MaxRange, 0, 1). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float DistanceNorm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	EWeaponType PlayerWeaponType = EWeaponType::Melee;

	/** W_norm: weapon threat lookup [0, 1]. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float WeaponThreatNorm = 0.f;

	/** H_norm = 1 - (CurrentHP / MaxHP). 0 = full HP, 1 = almost dead. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float EnemyHPRatioNorm = 0.f;

	/** A_norm = clamp(AllyCount / MaxAllyThreshold, 0, 1). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float AllyCountNorm = 0.f;

	/** R_norm = clamp(EntitiesInRoom / MaxRoomCapacity, 0, 1). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float RoomDensityNorm = 0.f;

	/** Raw count of nearby allies (pre-normalization, for debug). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	int32 NearbyAllyCount = 0;

	/** Snapshot of recent memory events for this evaluation tick. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	TArray<FMemoryEvent> RecentMemory;

	/** Aggregated decayed aggressiveness score from memory. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float MemoryAggressiveness = 0.f;

	/** Aggregated decayed mobility score from memory. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Context")
	float MemoryMobility = 0.f;
};

/**
 * FThreatAssessment
 * Layer 2 output: full threat computation result including confidence and smoothing.
 * Used by Layer 3 (StateTransitionResolver) for utility scoring.
 */
USTRUCT(BlueprintType)
struct FThreatAssessment
{
	GENERATED_BODY()

	/** T_base = Σ w_i * f_i(x_i) */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float ThreatBase = 0.f;

	/** T_cross = w_DH * f_D * f_H + w_WM * W * M_attack */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float ThreatCross = 0.f;

	/** T_raw = T_base + β * T_cross */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float ThreatRaw = 0.f;

	/** T_smooth = α * T_raw + (1-α) * T_prev */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float ThreatSmoothed = 0.f;

	/** T_final = C * T_smooth + (1-C) * T_default */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float ThreatFinal = 0.f;

	/** C = 1 / (1 + σ²). High variance → low confidence → conservative behavior. */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float Confidence = 1.f;

	/** Running mean of T_final (adaptive threshold). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float AdaptiveMeanThreat = 0.f;

	/** Running std dev (adaptive threshold). */
	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Threat")
	float AdaptiveStdDevThreat = 0.f;
};

/**
 * FStateScore
 * Per-state scoring breakdown used by UStateTransitionResolver.
 * Score(s) = UtilityValue - TransitionCost + InertiaBonus + PatternModifier.
 */
USTRUCT(BlueprintType)
struct FStateScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	EFSMStateType State = EFSMStateType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	float UtilityValue = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	float TransitionCost = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	float InertiaBonus = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	float PatternModifier = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Adaptive|Decision")
	float FinalScore = 0.f;
};

/** Delegates broadcast by UAdaptiveBehaviorComponent for logging/debug/UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatEvaluated,    const FThreatAssessment&, Assessment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatternRecognized,  const FString&,           Pattern);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAdaptiveDecisionMade, EFSMStateType, OldState, EFSMStateType, NewState);
