// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "StateTransitionResolver.generated.h"

class UAdaptiveConfig;
class UUtilityCurves;
class UTransitionCostMatrix;
class UPatternRecognizer;

/**
 * UStateTransitionResolver — Layer 3
 * Selects the optimal FSM state using utility scoring:
 *
 *   Score(s) = Utility(s, T_final)
 *            - CostMatrix[current][s]
 *            + Inertia(current, s, TimeInState)
 *            + PatternModifier(s)
 *
 *   s* = argmax(Score(s))
 *
 * Returns EFSMStateType::None if no change is recommended.
 * Implementation: Stage 6J.
 */
UCLASS()
class DEPTHRUN_API UStateTransitionResolver : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Selects the next FSM state. Returns current state if no transition wanted.
	 * All scoring breakdown is written to OutScores for debug widget.
	 */
	EFSMStateType ResolveNextState(
		EFSMStateType           CurrentState,
		const FThreatAssessment& Threat,
		const FContextData&     Context,
		float                   TimeInCurrentState,
		const UUtilityCurves*   UtilCurves,
		const UTransitionCostMatrix* CostMatrix,
		const UPatternRecognizer* PatternRecog,
		const UAdaptiveConfig*  Config,
		TArray<FStateScore>&    OutScores) const;

private:
	float ScoreState(
		EFSMStateType           Candidate,
		EFSMStateType           Current,
		float                   Utility,
		float                   Cost,
		float                   Inertia,
		float                   Pattern) const;
};
