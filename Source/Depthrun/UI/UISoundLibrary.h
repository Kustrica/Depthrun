// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UISoundLibrary.generated.h"

class USoundBase;

/**
 * UUISoundLibrary
 * GameInstanceSubsystem — holds UI SFX references and provides play helpers.
 * Assign sounds in BP_DepthrunGameInstance defaults (or via the subsystem BP class).
 *
 * Sounds:
 *   ButtonClick — plays on button left-click / confirm
 *   ButtonHover — plays on button mouse-over
 *   ChestOpen   — plays when chest reward scroll appears
 */
UCLASS()
class DEPTHRUN_API UUISoundLibrary : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Play button-click sound at 2D (no attenuation). */
	UFUNCTION(BlueprintCallable, Category = "UI|SFX")
	void PlayButtonClick();

	/** Play button-hover sound at 2D. */
	UFUNCTION(BlueprintCallable, Category = "UI|SFX")
	void PlayButtonHover();

	/** Play chest-open sound at 2D. */
	UFUNCTION(BlueprintCallable, Category = "UI|SFX")
	void PlayChestOpen();

	// ─── Assignable in Blueprint defaults ────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|SFX|Sounds")
	TObjectPtr<USoundBase> ButtonClickSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|SFX|Sounds")
	TObjectPtr<USoundBase> ButtonHoverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|SFX|Sounds")
	TObjectPtr<USoundBase> ChestOpenSound;

private:
	void PlaySound2D(USoundBase* Sound);
};
