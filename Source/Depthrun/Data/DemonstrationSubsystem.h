// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DemonstrationSubsystem.generated.h"

/**
 * UDemonstrationSubsystem
 * Diploma defense mode — provides step-by-step evaluation and CSV export.
 *
 * Features (Stage 11):
 *   - Time slow-down: slows game to 10% speed to show decision-making in action
 *   - Step mode: one adaptive evaluation per key press
 *   - CSV export: writes T_final, weights, state scores per evaluation tick
 *                 to Saved/AdaptiveBehaviorLog.csv for diploma charts
 *
 * Accessed via GetGameInstance()->GetSubsystem<UDemonstrationSubsystem>()
 */
UCLASS()
class DEPTHRUN_API UDemonstrationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ─── Demo Mode ───────────────────────────────────────────────────────────

	/** Enter slow-motion demonstration mode (TimeScale = 0.1). */
	UFUNCTION(BlueprintCallable, Category = "Demo")
	void EnterDemoMode();

	/** Exit demonstration mode, restore normal time scale. */
	UFUNCTION(BlueprintCallable, Category = "Demo")
	void ExitDemoMode();

	/** Toggle step-by-step mode: evaluations only proceed on RequestStep(). */
	UFUNCTION(BlueprintCallable, Category = "Demo")
	void SetStepMode(bool bEnabled);

	/** In step mode: trigger exactly one adaptive evaluation cycle. */
	UFUNCTION(BlueprintCallable, Category = "Demo")
	void RequestStep();

	/** True when demonstration mode is active. */
	UFUNCTION(BlueprintPure, Category = "Demo")
	bool IsInDemoMode() const { return bDemoModeActive; }

	// ─── CSV Export ──────────────────────────────────────────────────────────

	/** Begin recording evaluation data to CSV. */
	UFUNCTION(BlueprintCallable, Category = "Demo|CSV")
	void BeginCSVRecording(const FString& Filename = TEXT("AdaptiveBehaviorLog.csv"));

	/** Stop recording and flush the CSV file. */
	UFUNCTION(BlueprintCallable, Category = "Demo|CSV")
	void EndCSVRecording();

	/**
	 * Append one row to the CSV. Called by UAdaptiveBehaviorComponent each tick
	 * when recording is active.
	 */
	void RecordEvaluationRow(float TFinal, float Confidence, const TArray<float>& Weights,
		const FString& ChosenState, const FString& Pattern);

private:
	bool bDemoModeActive   = false;
	bool bStepModeEnabled  = false;
	bool bCSVRecording     = false;

	FString       CSVFilePath;
	TArray<FString> CSVRows;
};
