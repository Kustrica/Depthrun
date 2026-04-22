// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "AdaptiveBehavior/AdaptiveTypes.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "AdaptiveBehaviorComponent.generated.h"

class UAdaptiveConfig;
class UContextEvaluator;
class UThreatCalculator;
class UStateTransitionResolver;
class UAdaptiveMemory;
class UPatternRecognizer;
class UDynamicWeightManager;
class UUtilityCurves;
class UTransitionCostMatrix;
class UFSMComponent;

/**
 * UAdaptiveBehaviorComponent
 * Orchestrator of the 3-layer adaptive decision system.
 *
 * Pipeline (every EvaluationInterval seconds):
 *   Layer 1: ContextEvaluator  → FContextData
 *   Layer 2: ThreatCalculator  → FThreatAssessment (T_final, Confidence)
 *   Layer 3: StateTransitionResolver → s* = argmax(Score)
 *   → FSMComponent::TransitionTo(s*)
 *
 * Subscribes to:
 *   - PlayerActionTracker::OnPlayerAction → feeds Memory + PatternRecognizer
 *   - EnemyHealthComponent::OnDeath       → cleans up timer
 *   - damage taken/dealt delegates        → DynamicWeightManager reward signal
 */
UCLASS(ClassGroup = (Depthrun), meta = (BlueprintSpawnableComponent))
class DEPTHRUN_API UAdaptiveBehaviorComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UAdaptiveBehaviorComponent();

  // ─── Lifecycle ────────────────────────────────────────────────────────────

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // ─── Config (assign in Editor / DataAsset) ────────────────────────────────

  /** If false, the component will collect data but will NOT make FSM transitions (Layer 3 bypass). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
  bool bAdaptiveEnabled = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
  TObjectPtr<UAdaptiveConfig> Config;

  /** Character personality: affects how much threat is ignored. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive|Personality")
  EEnemyBravery BraveryLevel = EEnemyBravery::Normal;

  /** Combat preference: affects when the enemy switches between Melee and Ranged. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive|Personality")
  EEnemyCombatStyle CombatStyle = EEnemyCombatStyle::Balanced;

  // ─── Debug / Diploma readouts (all implemented in Stage 6L) ─────────────

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  float GetThreatFinal() const;

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  float GetConfidence() const;

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  TArray<float> GetCurrentWeights() const;

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  FString GetRecognizedPattern() const;

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  TArray<FStateScore> GetLastStateScores() const;

  UFUNCTION(BlueprintPure, Category = "Adaptive|Debug")
  FThreatAssessment GetLastThreatAssessment() const;

  // ─── Reward signals (called by damage delegates) ──────────────────────────

  /** Call when this enemy deals damage. Reward = +1. */
  UFUNCTION()
  void OnDamageDealt();

  /** Call when this enemy takes damage. Reward = -1. */
  UFUNCTION()
  void OnDamageTaken();

  /** Subscribed to PlayerActionTracker::OnPlayerAction. */
  UFUNCTION()
  void HandlePlayerAction(const FPlayerActionEvent &ActionEvent);

  // ─── Events (broadcast for debug widget / logging) ────────────────────────

  UPROPERTY(BlueprintAssignable, Category = "Adaptive|Events")
  FOnThreatEvaluated OnThreatEvaluated;

  UPROPERTY(BlueprintAssignable, Category = "Adaptive|Events")
  FOnPatternRecognized OnPatternRecognized;

  UPROPERTY(BlueprintAssignable, Category = "Adaptive|Events")
  FOnAdaptiveDecisionMade OnAdaptiveDecisionMade;

private:
  /** Core evaluation loop — called by FTimerManager at EvaluationInterval. */
  void EvaluationTick();

  void InitializeSubsystems();

  // ─── Subsystems (UObject-based, created in BeginPlay) ────────────────────

  UPROPERTY()
  TObjectPtr<UContextEvaluator> ContextEval;

  UPROPERTY()
  TObjectPtr<UThreatCalculator> ThreatCalc;

  UPROPERTY()
  TObjectPtr<UStateTransitionResolver> Resolver;

  UPROPERTY()
  TObjectPtr<UAdaptiveMemory> Memory;

  UPROPERTY()
  TObjectPtr<UPatternRecognizer> PatternRecog;

  UPROPERTY()
  TObjectPtr<UDynamicWeightManager> WeightManager;

  UPROPERTY()
  TObjectPtr<UUtilityCurves> UtilCurves;

  UPROPERTY()
  TObjectPtr<UTransitionCostMatrix> CostMatrix;

  /** Cached FSM component on the same actor. Set in BeginPlay. */
  UPROPERTY()
  TObjectPtr<UFSMComponent> FSMComp;

  FTimerHandle EvaluationTimerHandle;

  /** Cached from last evaluation for debug readouts. */
  FContextData LastContext;
  FThreatAssessment LastThreatAssessment;
  TArray<FStateScore> LastStateScores;
};
