// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/**
 * UHealthBarWidget
 * Reusable HP bar widget used for both player and enemies.
 * Layout is done in UMG Editor.
 * Implementation: Stage 9.
 */
UCLASS()
class DEPTHRUN_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Update the HP display. Call whenever HP changes.
	 * @param Current Current HP value.
	 * @param Max     Max HP value.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|Health")
	void SetHealthPercent(float Current, float Max);

	/** Returns current fill [0,1]. */
	UFUNCTION(BlueprintPure, Category = "UI|Health")
	float GetHealthPercent() const { return HealthPercent; }

protected:
	/** Called by SetHealthPercent after updating HealthPercent. Override in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Health")
	void OnHealthUpdated(float NewPercent);

private:
	float HealthPercent = 1.f;
};
