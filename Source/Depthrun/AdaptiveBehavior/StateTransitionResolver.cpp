// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "StateTransitionResolver.h"
#include "AdaptiveConfig.h"
#include "PatternRecognizer.h"
#include "TransitionCostMatrix.h"
#include "UtilityCurves.h"

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
	if (!UtilCurves || !CostMatrix || !PatternRecog) return CurrentState;

	const TArray<EFSMStateType> StatesToEvaluate = {
		EFSMStateType::Idle,
		EFSMStateType::Chase,
		EFSMStateType::Attack,
		EFSMStateType::Retreat,
		EFSMStateType::Flank
	};

	EFSMStateType BestState = CurrentState;
	float BestScore = -1e9f;

	for (EFSMStateType Candidate : StatesToEvaluate)
	{
		FStateScore Score;
		Score.State = Candidate;
		
		// 1. Utility from curves (Stage 6H + 6K Adaptive)
		Score.UtilityValue = UtilCurves->EvaluateUtility(Candidate, Threat, Context, Config);
		
		// 2. Transition cost from matrix (Stage 6I)
		Score.TransitionCost = CostMatrix->GetCost(CurrentState, Candidate);
		
		// 3. Inertia bonus (Stage 6I)
		Score.InertiaBonus = CostMatrix->CalculateInertia(CurrentState, Candidate, TimeInCurrentState, Config);
		
		// 4. Pattern modifier (Stage 6E)
		Score.PatternModifier = PatternRecog->GetPatternModifier(Candidate);
		
		// Final Score = Utility - Cost + Inertia + Pattern
		Score.FinalScore = ScoreState(Candidate, CurrentState, 
			Score.UtilityValue, Score.TransitionCost, Score.InertiaBonus, Score.PatternModifier);

		OutScores.Add(Score);

		if (Score.FinalScore > BestScore)
		{
			BestScore = Score.FinalScore;
			BestState = Candidate;
		}
	}

	return BestState;
}

float UStateTransitionResolver::ScoreState(
	EFSMStateType Candidate,
	EFSMStateType Current,
	float Utility, float Cost, float Inertia, float Pattern) const
{
	// Score(s) = Utility - Cost + Inertia + Pattern
	return Utility - Cost + Inertia + Pattern;
}
