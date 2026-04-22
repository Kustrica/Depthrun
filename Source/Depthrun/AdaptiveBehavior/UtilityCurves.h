// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "UtilityCurves.generated.h"

class UAdaptiveConfig;

/**
 * UUtilityCurves
 * Maps T_final → utility score [0,1] for each of the 5 FSM states.
 *
 * Formulas (from plan):
 *   Idle:    max(0, 1.0 - 4*T²)
 *   Chase:   BellCurve(T, center=0.3, width=0.2)
 *   Attack:  BellCurve(T, center=0.5, width=0.25)
 *   Flank:   BellCurve(T, center=0.6, width=0.2) * A_norm
 *   Retreat: Sigmoid(T, center=0.75, k=12)
 *
 * Curve parameters loaded from UAdaptiveConfig for editor tuning.
 * Implementation: Stage 6H.
 */
UCLASS()
class DEPTHRUN_API UUtilityCurves : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Evaluate utility of a given state for the current T_final and context.
	 * Context is needed for Flank (requires A_norm).
	 * Threat is needed for Adaptive Thresholds (Stage 6K).
	 */
	float EvaluateUtility(
		EFSMStateType           State,
		const FThreatAssessment& Threat,
		const FContextData&     Context,
		const UAdaptiveConfig*  Config) const;

private:
	float EvaluateIdle   (float T) const;
	float EvaluateChase  (float T, const UAdaptiveConfig* Cfg) const;
	float EvaluateAttack (float T, const UAdaptiveConfig* Cfg) const;
	float EvaluateFlank  (float T, float ANorm, const UAdaptiveConfig* Cfg) const;
	float EvaluateRetreat(float T, const UAdaptiveConfig* Cfg) const;
};
