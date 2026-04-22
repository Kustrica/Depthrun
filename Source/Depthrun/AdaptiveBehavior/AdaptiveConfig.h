// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AdaptiveConfig.generated.h"

/**
 * UAdaptiveConfig
 * Central DataAsset for all tuning parameters of the adaptive behavior module.
 *
 * Assign DA_AdaptiveConfig_Default to BP_AdaptiveEnemy.
 * All values are editable in the Details panel without recompilation.
 *
 * Covered by Stage 6A; pre-created here (Stage 4.5A) to allow Editor
 * asset creation before implementation begins.
 */
UCLASS(BlueprintType)
class DEPTHRUN_API UAdaptiveConfig : public UDataAsset {
  GENERATED_BODY()

public:
  // ─── Context Evaluation ──────────────────────────────────────────────────

  /** Maximum engagement distance (cm). Beyond this D_norm = 0. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Context")
  float MaxEngagementRange = 1200.f;

  /** Sphere radius for counting nearby allies. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Context")
  float AllyCheckRadius = 600.f;

  /** Ally count that maps to A_norm = 1.0. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Context")
  float MaxAllyThreshold = 4.f;

  /** Entity count in room that maps to R_norm = 1.0. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Context")
  float MaxRoomCapacity = 8.f;

  // ─── Non-linear Transforms ───────────────────────────────────────────────

  /** K parameter for sigmoid distance transform. Higher = sharper mid-range
   * spike. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|Transforms")
  float SigmoidSteepness_Distance = 10.f;

  /** α exponent for quadratic HP transform: f_H(x) = x^α. Default α=2. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|Transforms")
  float HealthPowerExponent = 2.f;

  // ─── Initial Weights w1..w6 ──────────────────────────────────────────────

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightDistance = 0.25f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightWeaponThreat = 0.2f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightHealth = 0.2f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightAllies = 0.15f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightRoomDensity = 0.1f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightMemory = 0.1f;

  // ─── Weight Adaptation ───────────────────────────────────────────────────

  /** η — learning rate for weight update rule. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightLearningRate = 0.04f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightMin = 0.05f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightMax = 0.4f;

  // ─── Cross Terms ─────────────────────────────────────────────────────────

  /** β for T_raw = T_base + β * T_cross. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  float CrossTermBeta = 0.2f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  float CrossWeightDistanceHealth = 0.15f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  float CrossWeightWeaponMemory = 0.1f;

  // ─── Smoothing & Confidence ──────────────────────────────────────────────

  /** α for exponential smoothing: T_smooth = α*T_raw + (1-α)*T_prev. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  float SmoothingAlpha = 0.3f;

  /** Window N for σ² computation (confidence). */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  int32 ConfidenceWindow = 8;

  /** Fallback threat when confidence is low (C → 0). */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|ThreatFormula")
  float DefaultThreat = 0.3f;

  // ─── Memory ──────────────────────────────────────────────────────────────

  /** λ for time-decay: M(t) = Intensity * exp(-λ * Δt). */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Memory")
  float MemoryDecayLambda = 0.3f;

  /** Maximum number of events in the ring buffer. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Memory")
  int32 MemoryBufferSize = 50;

  /** Events older than this (seconds) are pruned. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Memory")
  float MemoryMaxEventAge = 30.f;

  // ─── Pattern Recognition ─────────────────────────────────────────────────

  /** Sliding window for N-gram analysis. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Pattern")
  int32 PatternWindowSize = 15;

  // ─── Inertia ─────────────────────────────────────────────────────────────

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Inertia")
  float InertiaGrowthRate = 0.02f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Inertia")
  float InertiaMax = 0.2f;

  // ─── Adaptive Thresholds ─────────────────────────────────────────────────

  /** Rolling window K for adaptive threshold computation. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|Thresholds")
  int32 AdaptiveThresholdWindow = 20;

  // ─── Transition Cost Matrix [From][To] (5x5 =
  // None,Idle,Chase,Attack,Retreat,Flank) ── Stored flat [row*5 + col], indexed
  // by EFSMStateType cast to int (skip None=0) Init in DA asset or via code;
  // default values match the plan's 5x5 table.

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|CostMatrix")
  TArray<float> TransitionCostMatrix;

  // ─── Utility Curve Parameters (per state) ────────────────────────────────

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float ChaseBellCenter = 0.3f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float ChaseBellWidth = 0.2f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float AttackBellCenter = 0.5f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float AttackBellWidth = 0.25f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float FlankBellCenter = 0.6f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float FlankBellWidth = 0.2f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float RetreatSigmoidCenter = 0.75f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float RetreatSigmoidSteepness = 12.f;

  // ─── Evaluation Timer ────────────────────────────────────────────────────

  /** How often the adaptive module recalculates (seconds). Between 0.2 and 0.5.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Timing",
            meta = (ClampMin = "0.1", ClampMax = "1.0"))
  float EvaluationInterval = 0.3f;
};
