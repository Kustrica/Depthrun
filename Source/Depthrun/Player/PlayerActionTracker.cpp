// Copyright Depthrun Project, 2026. All Rights Reserved.

#include "PlayerActionTracker.h"
#include "Core/DepthrunLogChannels.h"

UPlayerActionTracker::UPlayerActionTracker()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerActionTracker::RecordAction(EPlayerActionType ActionType,
                                        const FVector& Location,
                                        float Intensity)
{
	FPlayerActionEvent Event;
	Event.ActionType = ActionType;
	Event.Timestamp  = GetCurrentTimestamp();
	Event.Location   = Location;
	Event.Intensity  = FMath::Clamp(Intensity, 0.f, 1.f);

	UE_LOG(LogDepthrun, Verbose, TEXT("PlayerActionTracker: action=%d t=%.2f"),
		static_cast<int32>(ActionType), Event.Timestamp);

	OnPlayerAction.Broadcast(Event);
}

float UPlayerActionTracker::GetCurrentTimestamp() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}
