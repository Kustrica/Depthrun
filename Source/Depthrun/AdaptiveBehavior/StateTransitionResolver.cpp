// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "StateTransitionResolver.h"

EFSMStateType UStateTransitionResolver::ResolveNextState(
	EFSMStateType           CurrentState,
	const FThreatAssessment& Threat,
	const FContextData&     Context,
	float                   TimeInCurrentState,
	const UUtilityCurves*   UtilCurves,
	const UTransitionCostMatrix* CostMatrix,
	const UPatternRecognizer* PatternRecog,
	const UAdaptiveConfig*  Config,
	TArray<FStateScore>&    OutScores) const
{
	OutScores.Reset();
	// TODO (Stage 6J): score all 5 states, populate OutScores, return argmax
	return CurrentState; // no-op until Stage 6J
}

float UStateTransitionResolver::ScoreState(
	EFSMStateType Candidate,
	EFSMStateType Current,
	float Utility, float Cost, float Inertia, float Pattern) const
{
	// Score(s) = Utility - Cost + Inertia + Pattern
	return Utility - Cost + Inertia + Pattern;
}
