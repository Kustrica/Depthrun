// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

/** Music tracks used by UMusicSubsystem. */
UENUM(BlueprintType)
enum class EMusicTrack : uint8
{
	None    UMETA(DisplayName = "None"),
	Hub     UMETA(DisplayName = "Hub"),
	Explore UMETA(DisplayName = "Explore"),
	Combat  UMETA(DisplayName = "Combat"),
};
