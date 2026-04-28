// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HubUpgradeTypes.h"
#include "DepthrunSaveSubsystem.generated.h"

class USQLiteManager;

/**
 * UDepthrunSaveSubsystem
 * Manages SQLite-backed persistence between runs.
 *
 * Tables: player_progress | run_history | adaptive_weights (for diploma charts)
 *
 * Accessed via GetGameInstance()->GetSubsystem<UDepthrunSaveSubsystem>()
 * Implementation: Stage 10.
 */
UCLASS()
class DEPTHRUN_API UDepthrunSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Save the result of one completed run. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveRunResult(int32 Floor, int32 Score, bool bWon);

	/** Load latest player progress. Returns false if no save exists. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadProgress(int32& OutBestFloor, int32& OutTotalRuns);

	/**
	 * Save current adaptive weights snapshot.
	 * Used to produce diploma charts showing weight evolution over time.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save|Diploma")
	void SaveAdaptiveWeights(const TArray<float>& Weights, int32 RunNumber);

	// ─── Player Profile (Meta-Progression) ───────────────────────────────────

	/** Get total persistent diamonds. */
	UFUNCTION(BlueprintPure, Category = "Save|Profile")
	int32 GetTotalDiamonds() const;

	/** Add diamonds to profile (e.g., 50% on death, 100% on Hub exit). */
	UFUNCTION(BlueprintCallable, Category = "Save|Profile")
	void AddDiamondsToProfile(int32 Amount);

	/** Get current upgrade level (0-5). */
	UFUNCTION(BlueprintPure, Category = "Save|Profile")
	int32 GetUpgradeLevel(EHubUpgrade Type) const;

	/** Get cost to upgrade to next level. Returns -1 if maxed. */
	UFUNCTION(BlueprintPure, Category = "Save|Profile")
	int32 GetUpgradeCost(EHubUpgrade Type) const;

	/** Try to buy upgrade. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "Save|Profile")
	bool BuyUpgrade(EHubUpgrade Type);

	/** Reset all profile data (for debug). */
	UFUNCTION(BlueprintCallable, Category = "Save|Debug")
	void ResetProfile();

private:
	void InitializeSchema();

	UPROPERTY()
	TObjectPtr<USQLiteManager> DB;
};
