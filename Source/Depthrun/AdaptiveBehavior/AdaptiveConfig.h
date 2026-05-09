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
  UAdaptiveConfig();

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

  // ─── HP Non-linear Transform ─────────────────────────────────────────────

  /** α exponent for quadratic HP transform: f_H(x) = (1-HP)^α. Default α=2.
   *  Higher values make low-HP spike threat more sharply. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|Transforms")
  float HealthPowerExponent = 2.f;

  // ─── Initial Weights w1..w6 ──────────────────────────────────────────────

  // Weights must sum to 1.0: 0.20 + 0.20 + 0.20 + 0.15 + 0.10 + 0.15 = 1.00
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightDistance = 0.20f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightWeaponThreat = 0.20f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightHealth = 0.20f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightAllies = 0.15f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightRoomDensity = 0.10f;

  /** Player aggressiveness memory — raised from 0.10 to ensure Memory
   *  visibly affects T_final during defense demonstration. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights",
            meta = (ClampMin = "0.05", ClampMax = "0.4"))
  float WeightMemory = 0.15f;

  // ─── Weight Adaptation ───────────────────────────────────────────────────

  /** η — learning rate for weight update rule.
   *  Raised from 0.04 to 0.10: at 0.04, weights needed 15-20 hits to show
   *  visible change — too slow for diploma defense demonstration.
   *  At 0.10, weights shift noticeably within 5-8 combat events. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightLearningRate = 0.10f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightMin = 0.05f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Weights")
  float WeightMax = 0.4f;

  // ─── Memory ──────────────────────────────────────────────────────────────

  /** Time window in seconds for counting player actions (aggressiveness counter).
   *  Lowered from 10 → 5: at 10s, 3 bow shots in 2s normalize to only 0.30,
   *  below the 0.4 Chase aggression-bonus threshold. At 5s the same input
   *  gives 0.60, making Memory visibly responsive to short spam bursts. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Memory")
  float MemoryWindowSeconds = 5.f;

  // ─── Pattern Recognition ─────────────────────────────────────────────────

  /** Sliding window for N-gram analysis. */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Pattern")
  int32 PatternWindowSize = 15;

  // ─── Inertia ─────────────────────────────────────────────────────────────

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Inertia")
  float InertiaGrowthRate = 0.02f;

  // Lowered from 0.2 to 0.1: at 0.2 cap enemy needed 10s in Chase before inertia
  // maxed out — but even 5s gave +0.10 bonus, enough to block pattern-driven transitions.
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Adaptive|Inertia")
  float InertiaMax = 0.1f;

  // ─── Transition Cost Matrix [From][To] (6×6) ────────────────────────────
  // States: None=0, Idle=1, Chase=2, Attack=3, Retreat=4, Flank=5
  // Stored flat: Cost = Matrix[From * 6 + To] where From/To = (int32)EFSMStateType
  // Row/col 0 (None) filled with 1.0 — prevents any transition to/from None.
  // Initialized in UAdaptiveConfig constructor; override values in DA asset.

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

  // Narrowed from 0.25 to 0.18: prevents Attack from dominating T=[0.35..0.65]
  // range which overlapped with the primary Flank activation zone.
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves")
  float AttackBellWidth = 0.18f;

  /** Base AllyFactor for Flank when enemy is solo (AllyCountNorm=0).
   *  Range [0..1]. At 1.0 solo Flank = full group Flank.
   *  Raised from 0.5 to 0.75: solo enemies must still be able to flank,
   *  otherwise Attack (peak=1.0) consistently outbids Flank (~0.50). */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
            Category = "Adaptive|UtilityCurves",
            meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float FlankSoloBase = 0.75f;

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
