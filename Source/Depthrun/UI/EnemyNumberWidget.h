// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyNumberWidget.generated.h"

/**
 * UEnemyNumberWidget
 * Tiny world-space widget displayed above each enemy.
 * Shows a short numeric ID (e.g. "#1", "#2") so you can correlate
 * the DebugAdaptiveWidget column with the actual enemy on screen.
 *
 * [HUMAN] Create WBP_EnemyNumber (child of UEnemyNumberWidget) in UMG:
 *   - Add a TextBlock named exactly "NumberText"
 *   - Font size 18–22, Bold, white with dark outline
 *   - Canvas size: ~60x30
 *   - No background needed (transparent)
 *
 * The C++ side sets the text via SetEnemyIndex().
 * The WidgetComponent is attached automatically in BaseEnemy constructor
 * if a widget class is assigned (BP_AdaptiveEnemy → EnemyNumberWidgetClass).
 */
UCLASS()
class DEPTHRUN_API UEnemyNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Set the displayed enemy index (0-based internally, shown as #1, #2, ...).
	 * Called from BaseEnemy::AssignDebugIndex().
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetEnemyIndex(int32 Index);

	/** Returns the currently displayed index (0-based). */
	UFUNCTION(BlueprintPure, Category = "Debug")
	int32 GetEnemyIndex() const { return EnemyIndex; }

protected:
	/** Called after SetEnemyIndex — override in Blueprint to animate or style. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void OnIndexSet(const FText& DisplayText);

private:
	int32 EnemyIndex = -1;
};
