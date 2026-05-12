#include "AdaptiveMemory.h"
#include "AdaptiveConfig.h"

void UAdaptiveMemory::Initialize(const UAdaptiveConfig* Config)
{
	if (Config) { MemoryWindowSeconds = Config->MemoryWindowSeconds; }
	MemoryBuffer.Reserve(MaxBufferSize);
}

void UAdaptiveMemory::RecordEvent(const FMemoryEvent& Event)
{
	if (MemoryBuffer.Num() >= MaxBufferSize)
	{
		// O(1) eviction: swap an arbitrary entry with last, then pop.
		// Order does not matter — CountWindowMetric filters by timestamp, not position.
		MemoryBuffer[0] = MemoryBuffer.Last();
		MemoryBuffer.Pop(EAllowShrinking::No);
	}
	MemoryBuffer.Add(Event);
}

// Lambda param kept for API compatibility — ignored in Variant E
float UAdaptiveMemory::GetDecayedAggressiveness(float CurrentTime, float /*Lambda*/) const
{
	return CountWindowMetric(CurrentTime, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Shot || T == EPlayerActionType::MeleeAttack;
	});
}

float UAdaptiveMemory::GetDecayedMobility(float CurrentTime, float /*Lambda*/) const
{
	return CountWindowMetric(CurrentTime, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Dash;
	});
}

float UAdaptiveMemory::GetDecayedCaution(float CurrentTime, float /*Lambda*/) const
{
	return CountWindowMetric(CurrentTime, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Heal;
	});
}

void UAdaptiveMemory::CleanupOldEvents(float CurrentTime, float /*MaxAge*/)
{
	// Always prune by the window (MaxAge param ignored in Variant E)
	MemoryBuffer.RemoveAll([CurrentTime, this](const FMemoryEvent& Ev)
	{
		return (CurrentTime - Ev.Timestamp) > MemoryWindowSeconds;
	});
}

float UAdaptiveMemory::CountWindowMetric(float CurrentTime,
	TFunctionRef<bool(EPlayerActionType)> Filter) const
{
	if (MemoryWindowSeconds <= 0.f) return 0.f;

	int32 Count = 0;
	for (const FMemoryEvent& Ev : MemoryBuffer)
	{
		if ((CurrentTime - Ev.Timestamp) <= MemoryWindowSeconds && Filter(Ev.ActionType))
		{
			++Count;
		}
	}
	// Normalize: 1 action/second = max aggressiveness (clamped to [0,1])
	return FMath::Clamp(static_cast<float>(Count) / MemoryWindowSeconds, 0.f, 1.f);
}
