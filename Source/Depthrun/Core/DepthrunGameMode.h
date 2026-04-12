// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DepthrunGameMode.generated.h"

/**
 * ADepthrunGameMode
 * Base game mode. Sets default classes and handles session-level state.
 */
UCLASS()
class DEPTHRUN_API ADepthrunGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADepthrunGameMode();

protected:
	virtual void BeginPlay() override;
};
