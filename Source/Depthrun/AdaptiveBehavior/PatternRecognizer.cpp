// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "PatternRecognizer.h"
#include "FSM/FSMTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogPatternRecognizer, Log, All);

void UPatternRecognizer::AddAction(EPlayerActionType ActionType)
{
	if (ActionWindow.Num() >= WindowSize)
	{
		ActionWindow.RemoveAt(0);
	}
	ActionWindow.Add(ActionType);
	RebuildNgrams();
}

FString UPatternRecognizer::GetDominantPattern() const
{
	// Find argmax over bigrams + trigrams (trigrams weighted x2 for specificity)
	FString BestPattern = TEXT("None");
	int32 BestCount = 0;

	for (const auto& Pair : Bigrams)
	{
		if (Pair.Value > BestCount)
		{
			BestCount = Pair.Value;
			BestPattern = Pair.Key;
		}
	}

	// Trigrams take priority if their count * 2 > current best (they encode more intent)
	for (const auto& Pair : Trigrams)
	{
		if (Pair.Value * 2 > BestCount)
		{
			BestCount = Pair.Value * 2;
			BestPattern = Pair.Key;
		}
	}

	UE_LOG(LogPatternRecognizer, Verbose, TEXT("[Pattern] dominant=%s count=%d"), *BestPattern, BestCount);
	return BestPattern;
}

float UPatternRecognizer::GetPatternModifier(EFSMStateType CandidateState) const
{
	const FString Dominant = GetDominantPattern();

	// Pattern → state modifier table (from architecture plan)
	// "Melee+Dash"  (aggressive rush)  → Retreat +0.2, Flank +0.15
	// "Dash+Melee"                     → Retreat +0.2, Flank +0.15
	// "Shot+Shot+Dash" (kiting)        → Chase +0.2
	// "Dash+Dash"    (evasive)         → Attack -0.1
	// "Shot+Shot"    (ranged pressure) → Retreat +0.1

	struct FPatternRule
	{
		FString Pattern;
		EFSMStateType State;
		float Modifier;
	};

	static const TArray<FPatternRule> Rules =
	{
		{ TEXT("Melee+Dash"),     EFSMStateType::Retreat, +0.20f },
		{ TEXT("Melee+Dash"),     EFSMStateType::Flank,   +0.15f },
		{ TEXT("Dash+Melee"),     EFSMStateType::Retreat, +0.20f },
		{ TEXT("Dash+Melee"),     EFSMStateType::Flank,   +0.15f },
		{ TEXT("Shot+Shot+Dash"), EFSMStateType::Chase,   +0.20f },
		{ TEXT("Dash+Dash"),      EFSMStateType::Attack,  -0.10f },
		{ TEXT("Shot+Shot"),      EFSMStateType::Retreat, +0.10f },
	};

	for (const FPatternRule& Rule : Rules)
	{
		if (Rule.Pattern == Dominant && Rule.State == CandidateState)
		{
			return Rule.Modifier;
		}
	}

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

	const int32 N = ActionWindow.Num();

	// Build 2-grams
	for (int32 i = 0; i + 1 < N; ++i)
	{
		FString Key = ActionToString(ActionWindow[i]) + TEXT("+") + ActionToString(ActionWindow[i + 1]);
		Bigrams.FindOrAdd(Key)++;
	}

	// Build 3-grams
	for (int32 i = 0; i + 2 < N; ++i)
	{
		FString Key = ActionToString(ActionWindow[i])
			+ TEXT("+") + ActionToString(ActionWindow[i + 1])
			+ TEXT("+") + ActionToString(ActionWindow[i + 2]);
		Trigrams.FindOrAdd(Key)++;
	}
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
