// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "PatternRecognizer.generated.h"

/**
 * UPatternRecognizer
 * N-gram frequency analysis of recent player actions.
 *
 * Stores last N=15 actions as a sliding window.
 * Counts 2-grams and 3-grams in a TMap<FString, int32>.
 * Returns the most frequent pattern and per-state utility modifiers.
 *
 * Pattern → modifier mapping (see plan):
 *   "Dash+MeleeAttack"   → Retreat +0.2, Flank +0.15
 *   "Shot+Shot+Dash"     → Chase +0.2
 *   "Dash+Dash"          → Attack -0.1
 *
 * Implementation: Stage 6E.
 */
struct FPatternRule
{
	FString Pattern;
	EFSMStateType State;
	float Modifier;
};

UCLASS()
class DEPTHRUN_API UPatternRecognizer : public UObject
{
	GENERATED_BODY()

public:
	/** Add the latest player action to the sliding window. */
	void AddAction(EPlayerActionType ActionType);

	/** Returns the most frequent N-gram pattern string, e.g. "Dash+MeleeAttack". */
	FString GetDominantPattern() const;

	/**
	 * Returns a score modifier [−0.3, +0.3] for the candidate FSM state
	 * based on the currently dominant pattern.
	 */
	float GetPatternModifier(EFSMStateType CandidateState) const;

	/** Clears the action window and frequency maps. */
	void Reset();

	int32 GetWindowSize() const { return WindowSize; }

private:
	void RebuildNgrams();
	FString ActionToString(EPlayerActionType T) const;

	/** Sliding action window (max WindowSize). */
	TArray<EPlayerActionType> ActionWindow;

	/** 1-gram frequency map. Key = "A". */
	TMap<FString, int32> Unigrams;

	/** 2-gram frequency map. Key = "A+B". */
	TMap<FString, int32> Bigrams;

	/** 3-gram frequency map. Key = "A+B+C". */
	TMap<FString, int32> Trigrams;

	int32 WindowSize = 15;
};
