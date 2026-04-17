// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerMovementConfig.generated.h"

/**
 * UPlayerMovementConfig
 * UDataAsset holding all player movement and dash tuning values.
 *
 * Assign DA_PlayerMovementConfig_Default to BP_DepthrunCharacter
 * to make all values editable in Details panel without recompilation.
 *
 * Stage 3E: read by ADepthrunCharacter::BeginPlay() to configure
 * UCharacterMovementComponent and dash behaviour.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API UPlayerMovementConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// ─── Normal Movement ─────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float MaxWalkSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float MaxAcceleration = 8192.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float BrakingDecelerationWalking = 8192.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float GroundFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float BrakingFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	float BrakingFrictionFactor = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Walking")
	bool bUseSeparateBrakingFriction = true;

	// ─── Dash ─────────────────────────────────────────────────────────────────

	/** Impulse magnitude applied at dash start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashImpulse = 800.f;

	/** Duration of the dash (seconds). Movement input locked for this duration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashDuration = 0.15f;

	/** Cooldown between dashes (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashCooldown = 1.0f;

	/**
	 * After DashDuration, velocity is multiplied by this factor to kill residual sliding.
	 * 0.0 = instant stop, 1.0 = no cancellation. Tune to taste.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PostDashVelocityCancelFactor = 0.1f;

	/** Velocity threshold below which post-dash stop is forced (cm/s). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashStopSpeedThreshold = 50.f;

	/** Delay before player input is restored after dash ends (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float InputRestoreDelay = 0.05f;
};
