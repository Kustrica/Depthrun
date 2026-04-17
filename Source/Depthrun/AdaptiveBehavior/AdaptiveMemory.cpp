// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveMemory.h"
#include "AdaptiveConfig.h"

void UAdaptiveMemory::Initialize(const UAdaptiveConfig* Config)
{
	if (Config) { MaxBufferSize = Config->MemoryBufferSize; }
	MemoryBuffer.Reserve(MaxBufferSize);
}

void UAdaptiveMemory::RecordEvent(const FMemoryEvent& Event)
{
	if (MemoryBuffer.Num() >= MaxBufferSize)
	{
		MemoryBuffer.RemoveAt(0); // evict oldest
	}
	MemoryBuffer.Add(Event);
}

float UAdaptiveMemory::GetDecayedAggressiveness(float CurrentTime, float Lambda) const
{
	return ComputeDecayedMetric(CurrentTime, Lambda, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Shot || T == EPlayerActionType::MeleeAttack;
	});
}

float UAdaptiveMemory::GetDecayedMobility(float CurrentTime, float Lambda) const
{
	return ComputeDecayedMetric(CurrentTime, Lambda, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Dash;
	});
}

float UAdaptiveMemory::GetDecayedCaution(float CurrentTime, float Lambda) const
{
	return ComputeDecayedMetric(CurrentTime, Lambda, [](EPlayerActionType T)
	{
		return T == EPlayerActionType::Heal;
	});
}

void UAdaptiveMemory::CleanupOldEvents(float CurrentTime, float MaxAge)
{
	MemoryBuffer.RemoveAll([CurrentTime, MaxAge](const FMemoryEvent& Ev)
	{
		return (CurrentTime - Ev.Timestamp) > MaxAge;
	});
}

float UAdaptiveMemory::ComputeDecayedMetric(float CurrentTime, float Lambda,
	TFunctionRef<bool(EPlayerActionType)> Filter) const
{
	float Total = 0.f;
	for (const FMemoryEvent& Ev : MemoryBuffer)
	{
		if (Filter(Ev.ActionType))
		{
			const float DeltaT = CurrentTime - Ev.Timestamp;
			Total += Ev.Intensity * FMath::Exp(-Lambda * DeltaT);
		}
	}
	return Total;
}
