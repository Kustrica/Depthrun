// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "MathUtils.h"
#include "Math/UnrealMathUtility.h"


namespace DepthrunMath
{
	float Sigmoid(float X, float K, float Center)
	{
		// 1 / (1 + e^(-K*(x-c)))
		return 1.f / (1.f + FMath::Exp(-K * (X - Center)));
	}

	float BellCurve(float X, float Center, float Width)
	{
		// exp(-(x-c)^2 / (2*w^2))
		const float Exponent = -FMath::Square(X - Center) / (2.f * FMath::Square(Width));
		return FMath::Exp(Exponent);
	}

	float TimeDecay(float BaseValue, float Lambda, float DeltaTime)
	{
		// M(t) = BaseValue * exp(-Lambda * DeltaTime)
		return BaseValue * FMath::Exp(-Lambda * DeltaTime);
	}

	float NormalizeToRange(float Value, float Min, float Max)
	{
		if (FMath::IsNearlyEqual(Min, Max))
		{
			return 0.f;
		}
		return FMath::Clamp((Value - Min) / (Max - Min), 0.f, 1.f);
	}
}
