// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HubWidget.generated.h"

class URunItemConfig;
class URunItemInventory;

/**
 * UHubWidget
 * Hub / Lobby screen between runs.
 * Content:
 *   - Run stats from last run (floor reached, kills)
 *   - Metaprogression: spend upgrade points (persistent, via DepthrunSaveSubsystem)
 *   - Item pool: pick up to MaxItems per run from available items
 *   - [Start Run] button → loads gameplay level
 *
 * Layout done in UMG Editor (WBP_HubWidget).
 * Implementation: Stage 9B.
 */
UCLASS()
class DEPTHRUN_API UHubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Populate the upgrade/item UI from save data. Call on widget creation. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void RefreshUI();

	/** Add displayed item to the run inventory (URunItemInventory on the GameInstance or pending state). */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void SelectItem(URunItemConfig* Config);

	/** Begin the run: clear inventory, load gameplay level. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void OnStartRunPressed();

	/** Return to Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void OnBackToMenuPressed();

	// ─── Config ──────────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub")
	FName GameplayLevelName = TEXT("L_Gameplay");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	/** All available items the player can choose from this run. Assigned in Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub|Items")
	TArray<TObjectPtr<URunItemConfig>> AvailableItems;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Hub")
	void OnUIRefreshed();

private:
	/** Items selected for the upcoming run (pending, applied on StartRun). */
	UPROPERTY()
	TArray<TObjectPtr<URunItemConfig>> SelectedItems;
};
