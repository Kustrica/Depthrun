// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "PatternRecognizer.h"

void UPatternRecognizer::AddAction(EPlayerActionType ActionType)
{
	if (ActionWindow.Num() >= WindowSize) { ActionWindow.RemoveAt(0); }
	ActionWindow.Add(ActionType);
	RebuildNgrams();
}

FString UPatternRecognizer::GetDominantPattern() const
{
	// TODO (Stage 6E): find max-count entry in Bigrams + Trigrams
	return TEXT("None");
}

float UPatternRecognizer::GetPatternModifier(EFSMStateType CandidateState) const
{
	// TODO (Stage 6E): map dominant pattern → per-state modifiers
	// "Dash+MeleeAttack" → Retreat +0.2, Flank +0.15
	// "Shot+Shot+Dash"   → Chase +0.2
	// "Dash+Dash"        → Attack -0.1
	return 0.f;
}

void UPatternRecognizer::Reset()
{
	ActionWindow.Reset();
	Bigrams.Reset();
	Trigrams.Reset();
}

void UPatternRecognizer::RebuildNgrams()
{
	Bigrams.Reset();
	Trigrams.Reset();
	// TODO (Stage 6E): rebuild frequency maps from ActionWindow
}

FString UPatternRecognizer::ActionToString(EPlayerActionType T) const
{
	switch (T)
	{
	case EPlayerActionType::Shot:           return TEXT("Shot");
	case EPlayerActionType::Dash:           return TEXT("Dash");
	case EPlayerActionType::MeleeAttack:    return TEXT("Melee");
	case EPlayerActionType::Heal:           return TEXT("Heal");
	case EPlayerActionType::SpecialAbility: return TEXT("Special");
	default:                                return TEXT("Unknown");
	}
}
