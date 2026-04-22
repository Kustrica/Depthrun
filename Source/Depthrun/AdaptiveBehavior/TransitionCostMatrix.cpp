// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "TransitionCostMatrix.h"
#include "AdaptiveConfig.h"

void UTransitionCostMatrix::InitializeFromConfig(const UAdaptiveConfig* Config)
{
	// Default 6×6 table matching EFSMStateType (None=0, Idle=1, Chase=2, Attack=3, Retreat=4, Flank=5)
	// None (Row 0) has high cost.
	for (int i = 0; i < 6; ++i) Matrix[0][i] = 1.0f;
	for (int i = 0; i < 6; ++i) Matrix[i][0] = 1.0f;

	const float Defaults[5][5] = {
		/*         Idle  Chase Attack Retreat Flank */
		/* Idle  */ { 0.00f, 0.05f, 0.10f,  0.05f, 0.15f },
		/* Chase */ { 0.10f, 0.00f, 0.05f,  0.15f, 0.10f },
		/* Atk   */ { 0.15f, 0.10f, 0.00f,  0.20f, 0.15f },
		/* Ret   */ { 0.10f, 0.15f, 0.20f,  0.00f, 0.25f },
		/* Flank */ { 0.15f, 0.10f, 0.10f,  0.15f, 0.00f },
	};

	for (int32 Row = 0; Row < 5; ++Row)
	{
		for (int32 Col = 0; Col < 5; ++Col)
		{
			Matrix[Row + 1][Col + 1] = Defaults[Row][Col];
		}
	}

	// Override from Config if valid (must be 6x6 = 36 elements)
	if (Config && Config->TransitionCostMatrix.Num() == 36)
	{
		for (int32 Row = 0; Row < 6; ++Row)
		{
			for (int32 Col = 0; Col < 6; ++Col)
			{
				Matrix[Row][Col] = Config->TransitionCostMatrix[Row * 6 + Col];
			}
		}
	}

	bInitialized = true;
}

float UTransitionCostMatrix::GetCost(EFSMStateType From, EFSMStateType To) const
{
	if (!bInitialized) return 0.f;
	const int32 F = static_cast<int32>(From);
	const int32 T = static_cast<int32>(To);
	if (F < 0 || F >= 6 || T < 0 || T >= 6) return 0.f;
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
	return static_cast<int32>(State);
}
