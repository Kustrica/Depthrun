// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatMusicTrigger.generated.h"

/**
 * UCombatMusicTrigger
 * ActorComponent attached to ADepthrunCharacter.
 * Every 0.5s checks whether the current active room has living enemies.
 * Switches music:  enemies alive → Combat,  no enemies → Explore.
 * Uses a 1.5s debounce before transitioning away from Combat
 * (avoids flickering when enemies briefly disappear between rooms).
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API UCombatMusicTrigger : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatMusicTrigger();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	/** Timer callback — evaluates combat state. */
	void EvaluateCombatState();

	FTimerHandle EvalTimerHandle;

	/** How long to wait after combat ends before switching back to Explore. */
	UPROPERTY(EditDefaultsOnly, Category = "Music", meta = (ClampMin = "0.0"))
	float CombatEndDebounce = 1.5f;

	/** How often to poll (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Music", meta = (ClampMin = "0.1"))
	float PollInterval = 0.5f;

	/** World time when combat last ended. Used for debounce. */
	float CombatEndTime = -999.f;

	bool bWasInCombat = false;
};
