// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "DebugAdaptiveWidget.generated.h"

class UAdaptiveBehaviorComponent;

/**
 * UDebugAdaptiveWidget
 * Real-time debug overlay displayed during diploma defense.
 * Shows: current FSM state, T_final (bar + value), Confidence,
 *        current weights (6-bar histogram), recognized pattern,
 *        utility of each state (5 bars).
 *
 * MANDATORY for diploma: committee must see the module working in real time.
 *
 * Layout: done in UMG Editor (WBP_DebugAdaptiveWidget Blueprint child).
 * Logic:  implemented in Stage 7.
 */
UCLASS()
class DEPTHRUN_API UDebugAdaptiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bind to a specific enemy's adaptive component. Call after widget creation. */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void BindToComponent(UAdaptiveBehaviorComponent* Component);

	/** Poll adaptive component data and refresh all display elements. */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void RefreshDisplay();

	// ─── Data accessors (called by UMG bindings or Refresh) ──────────────────

	UFUNCTION(BlueprintPure, Category = "Debug")
	float GetThreatFinal() const;

	UFUNCTION(BlueprintPure, Category = "Debug")
	float GetConfidence() const;

	UFUNCTION(BlueprintPure, Category = "Debug")
	TArray<float> GetWeights() const;

	UFUNCTION(BlueprintPure, Category = "Debug")
	FString GetPatternString() const;

	UFUNCTION(BlueprintPure, Category = "Debug")
	TArray<FStateScore> GetStateScores() const;

protected:
	/** Override in Blueprint to update individual UMG elements. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void OnDisplayRefreshed();

private:
	UPROPERTY()
	TObjectPtr<UAdaptiveBehaviorComponent> BoundComponent;
};
