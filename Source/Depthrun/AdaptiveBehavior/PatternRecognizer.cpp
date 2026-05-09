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
	// Stage 7.1 Implementation: Weighting Trigrams > Bigrams > Unigrams.
	FString BestPattern = TEXT("None");
	int32 BestScore = 0;

	for (const auto& Pair : Unigrams)
	{
		if (Pair.Value > BestScore)
		{
			BestScore = Pair.Value;
			BestPattern = Pair.Key;
		}
	}

	for (const auto& Pair : Bigrams)
	{
		if (Pair.Value * 2 > BestScore)
		{
			BestScore = Pair.Value * 2;
			BestPattern = Pair.Key;
		}
	}

	for (const auto& Pair : Trigrams)
	{
		if (Pair.Value * 3 > BestScore)
		{
			BestScore = Pair.Value * 3;
			BestPattern = Pair.Key;
		}
	}

	return BestPattern;
}

float UPatternRecognizer::GetPatternModifier(EFSMStateType CandidateState) const
{
	const FString Dominant = GetDominantPattern();

	static const TArray<FPatternRule> Rules =
	{
		// ── Trigrams ─────────────────────────────────────────────────────────
		// 3x Shot in a row → very strong Flank pressure (enemy must circle player)
		{ TEXT("Shot+Shot+Shot"), EFSMStateType::Flank,   +0.65f },
		{ TEXT("Shot+Shot+Shot"), EFSMStateType::Chase,   -0.20f }, // suppress frontal rush
		{ TEXT("Shot+Shot+Shot"), EFSMStateType::Attack,  -0.25f }, // attacking into sustained fire is suicidal
		{ TEXT("Shot+Shot+Dash"), EFSMStateType::Flank,   +0.45f },
		{ TEXT("Shot+Shot+Dash"), EFSMStateType::Chase,   +0.10f }, // was +0.35 — now Flank wins
		{ TEXT("Melee+Dash"),     EFSMStateType::Retreat, +0.25f },
		{ TEXT("Melee+Dash"),     EFSMStateType::Flank,   +0.15f },
		{ TEXT("Dash+Melee"),     EFSMStateType::Retreat, +0.25f },
		{ TEXT("Dash+Melee"),     EFSMStateType::Flank,   +0.15f },
		{ TEXT("Dash+Dash"),      EFSMStateType::Attack,  -0.15f },
		{ TEXT("Dash+Dash"),      EFSMStateType::Chase,   +0.20f },

		// ── Bigrams ──────────────────────────────────────────────────────────
		// 2x Shot → clear Flank signal; already on 2nd arrow enemy starts circling
		{ TEXT("Shot+Shot"),      EFSMStateType::Flank,   +0.65f },
		{ TEXT("Shot+Shot"),      EFSMStateType::Chase,   -0.10f }, // suppress head-on
		{ TEXT("Shot+Shot"),      EFSMStateType::Retreat, +0.10f },
		// Running into ranged fire is tactically wrong: suppress Attack when player shoots
		{ TEXT("Shot+Shot"),      EFSMStateType::Attack,  -0.20f },

		// ── Unigrams (spam counters) ─────────────────────────────────────────
		// Single Shot unigram: Flank >> Chase so enemy doesn't just run straight at player.
		// Flank +0.50 (was +0.40) — guarantees Flank wins even from a single bow shot
		// when Attack is far from peak (e.g. T<0.35 or T>0.65). Combined with the
		// relaxed close-range penalty, committee can demonstrate Flank inside 1-2 shots.
		{ TEXT("Melee"),          EFSMStateType::Flank,   +0.30f },
		{ TEXT("Melee"),          EFSMStateType::Retreat, +0.10f },
		{ TEXT("Shot"),           EFSMStateType::Flank,   +0.50f },
		{ TEXT("Shot"),           EFSMStateType::Chase,   +0.10f }, // was +0.25 — Chase no longer competes with Flank under bow pressure
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
	Unigrams.Reset();
	Bigrams.Reset();
	Trigrams.Reset();
}

void UPatternRecognizer::RebuildNgrams()
{
	Unigrams.Reset();
	Bigrams.Reset();
	Trigrams.Reset();

	const int32 N = ActionWindow.Num();

	// Build 1-grams
	for (int32 i = 0; i < N; ++i)
	{
		Unigrams.FindOrAdd(ActionToString(ActionWindow[i]))++;
	}

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
