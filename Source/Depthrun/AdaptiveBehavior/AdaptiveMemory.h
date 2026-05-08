// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "AdaptiveMemory.generated.h"

class UAdaptiveConfig;

/**
 * UAdaptiveMemory
 * Ring buffer of player actions with simple time-window counting (Variant E).
 *
 * Formula: Aggressiveness = count(Shot + Melee in last W seconds) / W
 *   W = MemoryWindowSeconds (default 10s)
 *
 * Three aggregated metrics for Layer 1:
 *   Aggressiveness = (Shot + MeleeAttack count) / MemoryWindowSeconds, clamped [0,1]
 *   Mobility       = Dash count / MemoryWindowSeconds, clamped [0,1]
 *   Caution        = Heal count / MemoryWindowSeconds, clamped [0,1]
 *
 * Old exponential-decay API (GetDecayedAggressiveness etc.) is preserved
 * but now delegates to the counter implementation.
 *
 * Implementation: Stage 6D (Variant E refactor).
 */
UCLASS()
class DEPTHRUN_API UAdaptiveMemory : public UObject
{
	GENERATED_BODY()

public:
	/** Add a new event to the ring buffer. Evicts oldest if at capacity. */
	void RecordEvent(const FMemoryEvent& Event);

	/**
	 * Count of aggressive actions (Shot, MeleeAttack) in the recent window,
	 * normalized by window length → [0, 1].
	 * Lambda param kept for API compatibility but ignored.
	 */
	float GetDecayedAggressiveness(float CurrentTime, float Lambda) const;

	/** Count of mobility actions (Dash) in window, normalized [0,1]. */
	float GetDecayedMobility(float CurrentTime, float Lambda) const;

	/** Count of cautious actions (Heal) in window, normalized [0,1]. */
	float GetDecayedCaution(float CurrentTime, float Lambda) const;

	/** Remove all events outside the MemoryWindowSeconds window. */
	void CleanupOldEvents(float CurrentTime, float MaxAge);

	/** Read-only access for debug widget. */
	const TArray<FMemoryEvent>& GetBuffer() const { return MemoryBuffer; }

	void Initialize(const UAdaptiveConfig* Config);

private:
	/** Count actions matching Filter within the time window, normalized. */
	float CountWindowMetric(float CurrentTime,
		TFunctionRef<bool(EPlayerActionType)> Filter) const;

	UPROPERTY()
	TArray<FMemoryEvent> MemoryBuffer;

	int32 MaxBufferSize = 200;

	/** Time window in seconds for counting (set from Config). */
	float MemoryWindowSeconds = 10.f;
};
