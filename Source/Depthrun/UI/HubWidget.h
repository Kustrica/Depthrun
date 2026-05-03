// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/HubUpgradeTypes.h"
#include "HubWidget.generated.h"

class URunItemInventory;
class URunItemCollection;
class UDepthrunSaveSubsystem;

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

	/** Add displayed item by index from ItemCollection to the pending run inventory. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void SelectItem(int32 ItemIndex);

	/** Begin the run: clear inventory, load gameplay level. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void OnStartRunPressed();

	/** Return to Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "Hub")
	void OnBackToMenuPressed();

	// ─── 9G: Metaprogression API (callable from Blueprint) ───────────────────

	/** Returns total diamonds in player profile. */
	UFUNCTION(BlueprintPure, Category = "Hub|Upgrades")
	int32 GetTotalDiamonds() const;

	/** Returns current level (0-5) for a given upgrade type. */
	UFUNCTION(BlueprintPure, Category = "Hub|Upgrades")
	int32 GetUpgradeLevel(EHubUpgrade Type) const;

	/** Returns cost of next level for upgrade. -1 if maxed. */
	UFUNCTION(BlueprintPure, Category = "Hub|Upgrades")
	int32 GetUpgradeCost(EHubUpgrade Type) const;

	/** Try to buy upgrade. Calls RefreshUI on success. */
	UFUNCTION(BlueprintCallable, Category = "Hub|Upgrades")
	void OnUpgradePressed(EHubUpgrade Type);

	// ─── Config ──────────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub")
	FName GameplayLevelName = TEXT("L_Gameplay");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	/** All available run items. Assign DA_RunItemCollection. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub|Items")
	TObjectPtr<URunItemCollection> ItemCollection;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Hub")
	void OnUIRefreshed();

private:
	/** Indices into ItemCollection.Items selected for the upcoming run. */
	TArray<int32> SelectedItemIndices;
};
