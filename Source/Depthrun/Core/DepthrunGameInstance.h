// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DepthrunGameInstance.generated.h"

/**
 * UDepthrunGameInstance
 * Persists across level transitions. Holds session-scope state
 * (e.g. current floor, run stats) accessible to all systems.
 */
UCLASS()
class DEPTHRUN_API UDepthrunGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDepthrunGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	/** Current dungeon floor number (1-based). */
	UPROPERTY(BlueprintReadOnly, Category = "Depthrun|Session")
	int32 CurrentFloor = 1;

	/** Total enemies killed this run. */
	UPROPERTY(BlueprintReadOnly, Category = "Depthrun|Session")
	int32 TotalKillsThisRun = 0;
};
