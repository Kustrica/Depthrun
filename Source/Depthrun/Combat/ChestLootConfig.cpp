// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Combat/ChestLootConfig.h"
#include "Math/UnrealMathUtility.h"

UChestLootConfig::UChestLootConfig()
{
	// Defaults are already set via UPROPERTY initializers above.
	// No extra setup needed.
}

int32 UChestLootConfig::RollDiamonds() const
{
	return FMath::RandRange(DiamondMin, DiamondMax);
}
