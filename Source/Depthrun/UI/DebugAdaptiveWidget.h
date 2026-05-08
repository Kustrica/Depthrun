// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "FSM/FSMTypes.h"
#include "DebugAdaptiveWidget.generated.h"

class UAdaptiveBehaviorComponent;
class AAdaptiveEnemy;

/**
 * FEnemyDebugSnapshot
 * All data for one enemy column, polled every RefreshInterval seconds.
 * Passed to Blueprint via GetEnemySnapshot(Index).
 */
USTRUCT(BlueprintType)
struct FEnemyDebugSnapshot
{
	GENERATED_BODY()

	/** True if this slot has a live enemy. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	bool bValid = false;

	/** Short display name, e.g. "Enemy_1". */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FString EnemyName;

	/** Slot index (0-based). Matches the #N label shown above the enemy in world. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	int32 EnemyIndex = -1;

	// ─── Threat pipeline ────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float ThreatFinal = 0.f;

	/** Confidence is always 1.0 in Variant E — shown for context. */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float Confidence = 1.f;

	// ─── Context factors (what feeds into T_final) ───────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float DistanceNorm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float EnemyHPNorm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float WeaponThreatNorm = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float MemoryAggressiveness = 0.f;

	// ─── Dynamic weights (6 values, indices 0..5) ───────────────────────────

	/** Current dynamic weights: [Distance, Weapon, Health, Allies, Density, Memory] */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<float> Weights;

	// ─── State scoring (Layer 3) ─────────────────────────────────────────────

	/** FinalScore for each of 5 states [Idle, Chase, Attack, Retreat, Flank] */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	TArray<FStateScore> StateScores;

	// ─── Decision ────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FString CurrentFSMState;

	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FString RecognizedPattern;

	/** True = ranged, False = melee */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	bool bRangedMode = false;

	/** Personality: Coward/Normal/Brave/Heroic */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FString BraveryLabel;

	/** CombatStyle: Melee/Balanced/Ranged */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	FString CombatStyleLabel;

	/** Distance to player in world units (raw). */
	UPROPERTY(BlueprintReadOnly, Category = "Debug")
	float DistanceRaw = 0.f;
};

/**
 * UDebugAdaptiveWidget
 * Tracks up to 4 AAdaptiveEnemy actors automatically (no manual binding needed).
 * Polls them every RefreshInterval seconds and exposes FEnemyDebugSnapshot per slot.
 *
 * Blueprint usage:
 *   - Create WBP_DebugAdaptiveWidget extending this class
 *   - In Event Tick (or timer): call RefreshDisplay()
 *   - Build 4 column panels, each driven by GetEnemySnapshot(0..3)
 *   - Display fields from FEnemyDebugSnapshot directly (no custom binding needed)
 *
 * MANDATORY for diploma defense — committee must see adaptive module in real time.
 */
UCLASS()
class DEPTHRUN_API UDebugAdaptiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ─── Configuration ───────────────────────────────────────────────────────

	/** How often enemy list is re-scanned (seconds). Keep > EvaluationInterval. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Config")
	float ScanInterval = 2.0f;

	/** Max number of enemy columns displayed. Hard-capped at 4. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Config",
	          meta = (ClampMin = "1", ClampMax = "4"))
	int32 MaxTrackedEnemies = 4;

	// ─── Main API ────────────────────────────────────────────────────────────

	/**
	 * Re-poll all tracked enemies and update all snapshots.
	 * Call from Blueprint Event Tick (or bind to a timer).
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void RefreshDisplay();

	/**
	 * Force an immediate enemy scan + snapshot update, bypassing ScanInterval timer.
	 * Called by HUD when the debug widget is first shown (F3 open) so that #N labels
	 * appear instantly without waiting up to ScanInterval seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ForceRefresh();

	/**
	 * Get snapshot for column Index (0..3).
	 * If bValid == false, the slot is empty — hide the column in UMG.
	 */
	UFUNCTION(BlueprintPure, Category = "Debug")
	FEnemyDebugSnapshot GetEnemySnapshot(int32 Index) const;

	/** Number of currently tracked live enemies. */
	UFUNCTION(BlueprintPure, Category = "Debug")
	int32 GetTrackedEnemyCount() const;

	// ─── Legacy single-enemy API (kept for old Blueprint nodes) ─────────────

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void BindToComponent(UAdaptiveBehaviorComponent* Component);

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

	UFUNCTION(BlueprintPure, Category = "Debug")
	FText GetGodModeText() const;

	UFUNCTION(BlueprintPure, Category = "Debug")
	FText GetSuperAttackText() const;

protected:
	virtual void NativeConstruct() override;

	/** Called after every RefreshDisplay(). Override in Blueprint to animate updates. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void OnDisplayRefreshed();

private:
	/** Scan world for AAdaptiveEnemy actors and fill TrackedEnemies. */
	void ScanForEnemies();

	/** Build a snapshot from a live enemy. */
	FEnemyDebugSnapshot BuildSnapshot(AAdaptiveEnemy* Enemy) const;

	/** Convert EEnemyBravery to display string. */
	static FString BraveryToString(EEnemyBravery Bravery);

	/** Convert EEnemyCombatStyle to display string. */
	static FString CombatStyleToString(EEnemyCombatStyle Style);

	/** Up to MaxTrackedEnemies live enemies found in the world. */
	UPROPERTY()
	TArray<TObjectPtr<AAdaptiveEnemy>> TrackedEnemies;

	/** One snapshot per tracked enemy slot (always MaxTrackedEnemies entries). */
	TArray<FEnemyDebugSnapshot> Snapshots;

	float TimeSinceLastScan = 0.f;

	// Legacy single-enemy binding
	UPROPERTY()
	TObjectPtr<UAdaptiveBehaviorComponent> BoundComponent;
};
