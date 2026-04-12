// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

/**
 * DepthrunMath
 * Static math helpers used by the AdaptiveBehavior module.
 * All functions are formula-driven and academically defendable.
 *
 * Sigmoid:   maximum sensitivity at center, tapering at extremes.
 * BellCurve: Gaussian-shaped utility function for state scoring.
 * TimeDecay: exponential decay M(t) = Base * exp(-Lambda * DeltaTime).
 * Normalize: linear clamp-normalize to [0, 1].
 */
namespace DepthrunMath
{
	/**
	 * Logistic sigmoid.
	 * f(x) = 1 / (1 + exp(-K * (x - Center)))
	 * Default: K=10, Center=0.5  — max sensitivity at mid-range.
	 */
	float Sigmoid(float X, float K = 10.f, float Center = 0.5f);

	/**
	 * Gaussian bell curve (utility function).
	 * f(x) = exp(-(x - Center)^2 / (2 * Width^2))
	 */
	float BellCurve(float X, float Center, float Width);

	/**
	 * Exponential time decay.
	 * M(t) = BaseValue * exp(-Lambda * DeltaTime)
	 * Lambda=0.3: event 3s ago retains ~40%, 10s ago ~5%.
	 */
	float TimeDecay(float BaseValue, float Lambda, float DeltaTime);

	/**
	 * Linearly normalize Value from [Min, Max] to [0, 1].
	 * Clamped: returns 0 if Value <= Min, 1 if Value >= Max.
	 */
	float NormalizeToRange(float Value, float Min, float Max);
}
