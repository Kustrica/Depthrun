// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.generated.h"

/**
 * ERoomType
 * Used by ARoomBase and URoomGeneratorSubsystem to classify rooms.
 * Placed here (Enemy module) rather than RoomGeneration to allow
 * enemy spawning logic to reference room types without reverse deps.
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Rest   UMETA(DisplayName = "Rest"),
	Boss   UMETA(DisplayName = "Boss"),
	Start  UMETA(DisplayName = "Start")
};

/**
 * EEnemySpawnState
 * Tracks per-spawn-point occupancy during room activation.
 */
UENUM(BlueprintType)
enum class EEnemySpawnState : uint8
{
	Free     UMETA(DisplayName = "Free"),
	Occupied UMETA(DisplayName = "Occupied")
};
