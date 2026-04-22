// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FSM/FSMTypes.h"
#include "TransitionCostMatrix.generated.h"

class UAdaptiveConfig;

/**
 * UTransitionCostMatrix
 * 5×5 cost matrix penalising rapid state switches.
 *
 * Indexed by (From, To) cast to int32, skipping EFSMStateType::None.
 * Index mapping: Idle=0, Chase=1, Attack=2, Retreat=3, Flank=4
 *
 * Default values (from plan):
 *         Idle  Chase  Attack  Retreat  Flank
 * Idle    0.00   0.05   0.10    0.05    0.15
 * Chase   0.10   0.00   0.05    0.15    0.10
 * Attack  0.15   0.10   0.00    0.20    0.15
 * Retreat 0.10   0.15   0.20    0.00    0.25
 * Flank   0.15   0.10   0.10    0.15    0.00
 *
 * Inertia: min(InertiaMax, GrowthRate * TimeInState) — added when staying.
 *
 * Implementation: Stage 6I.
 */
UCLASS()
class DEPTHRUN_API UTransitionCostMatrix : public UObject
{
	GENERATED_BODY()

public:
	/** Populate matrix from Config or use plan defaults. Call once in BeginPlay. */
	void InitializeFromConfig(const UAdaptiveConfig* Config);

	/** Get transition cost from one state to another. Returns 0 for same state. */
	float GetCost(EFSMStateType From, EFSMStateType To) const;

	/**
	 * Inertia bonus for staying in the current state.
	 * Applied only when Candidate == Current.
	 * = min(InertiaMax, InertiaGrowthRate * TimeInState)
	 */
	float CalculateInertia(EFSMStateType Current, EFSMStateType Candidate,
		float TimeInState, const UAdaptiveConfig* Config) const;

private:
	int32 StateToIndex(EFSMStateType State) const;

	/** Flat 6×6 matrix [row * 6 + col]. Row = From, Col = To. */
	float Matrix[6][6] = {};
	bool bInitialized = false;
};
