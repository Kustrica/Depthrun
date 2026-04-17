// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "TransitionCostMatrix.h"
#include "AdaptiveConfig.h"

void UTransitionCostMatrix::InitializeFromConfig(const UAdaptiveConfig* Config)
{
	// Default 5×5 table from plan (Idle=0,Chase=1,Attack=2,Retreat=3,Flank=4)
	const float Defaults[5][5] = {
		/*         Idle  Chase Attack Retreat Flank */
		/* Idle  */ { 0.00f, 0.05f, 0.10f,  0.05f, 0.15f },
		/* Chase */ { 0.10f, 0.00f, 0.05f,  0.15f, 0.10f },
		/* Atk   */ { 0.15f, 0.10f, 0.00f,  0.20f, 0.15f },
		/* Ret   */ { 0.10f, 0.15f, 0.20f,  0.00f, 0.25f },
		/* Flank */ { 0.15f, 0.10f, 0.10f,  0.15f, 0.00f },
	};
	FMemory::Memcpy(Matrix, Defaults, sizeof(Matrix));

	// TODO (Stage 6I): override from Config->TransitionCostMatrix if populated
	bInitialized = true;
}

float UTransitionCostMatrix::GetCost(EFSMStateType From, EFSMStateType To) const
{
	if (!bInitialized) return 0.f;
	const int32 F = StateToIndex(From);
	const int32 T = StateToIndex(To);
	if (F < 0 || T < 0) return 0.f;
	return Matrix[F][T];
}

float UTransitionCostMatrix::CalculateInertia(EFSMStateType Current, EFSMStateType Candidate,
	float TimeInState, const UAdaptiveConfig* Config) const
{
	if (Current != Candidate) return 0.f;
	const float Rate = Config ? Config->InertiaGrowthRate : 0.02f;
	const float Max  = Config ? Config->InertiaMax        : 0.2f;
	return FMath::Min(Max, Rate * TimeInState);
}

int32 UTransitionCostMatrix::StateToIndex(EFSMStateType State) const
{
	switch (State)
	{
	case EFSMStateType::Idle:    return 0;
	case EFSMStateType::Chase:   return 1;
	case EFSMStateType::Attack:  return 2;
	case EFSMStateType::Retreat: return 3;
	case EFSMStateType::Flank:   return 4;
	default:                     return -1;
	}
}
