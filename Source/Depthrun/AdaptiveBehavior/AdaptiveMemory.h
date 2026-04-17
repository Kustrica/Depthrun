// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "AdaptiveMemory.generated.h"

class UAdaptiveConfig;

/**
 * UAdaptiveMemory
 * Ring buffer of player actions with exponential time-decay retrieval.
 *
 * Formula: M_i(t) = Intensity_i * exp(-λ * (t_current - t_event))
 *   λ = 0.3 → event 3s ago retains ~40%, 10s ago ~5%
 *
 * Three aggregated metrics for Layer 1:
 *   Aggressiveness = Σ M_i  for ActionType ∈ {Shot, MeleeAttack}
 *   Mobility       = Σ M_i  for ActionType ∈ {Dash}
 *   Caution        = Σ M_i  for ActionType ∈ {Heal, Retreat}
 *
 * Implementation: Stage 6D.
 */
UCLASS()
class DEPTHRUN_API UAdaptiveMemory : public UObject
{
	GENERATED_BODY()

public:
	/** Add a new event to the ring buffer. Evicts oldest if at capacity. */
	void RecordEvent(const FMemoryEvent& Event);

	/** Σ decayed intensity for aggressive actions (Shot, MeleeAttack). */
	float GetDecayedAggressiveness(float CurrentTime, float Lambda) const;

	/** Σ decayed intensity for mobility actions (Dash). */
	float GetDecayedMobility(float CurrentTime, float Lambda) const;

	/** Σ decayed intensity for cautious actions (Heal). */
	float GetDecayedCaution(float CurrentTime, float Lambda) const;

	/** Remove all events older than MaxAge seconds. */
	void CleanupOldEvents(float CurrentTime, float MaxAge);

	/** Read-only access for debug widget. */
	const TArray<FMemoryEvent>& GetBuffer() const { return MemoryBuffer; }

	void Initialize(const UAdaptiveConfig* Config);

private:
	float ComputeDecayedMetric(float CurrentTime, float Lambda,
		TFunctionRef<bool(EPlayerActionType)> Filter) const;

	UPROPERTY()
	TArray<FMemoryEvent> MemoryBuffer;

	int32 MaxBufferSize = 50;
};
