// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveBehaviorComponent.h"
#include "AdaptiveConfig.h"
#include "AdaptiveMemory.h"
#include "ContextEvaluator.h"
#include "DepthrunLogChannels.h"
#include "DynamicWeightManager.h"
#include "FSM/FSMComponent.h"
#include "PatternRecognizer.h"
#include "StateTransitionResolver.h"
#include "ThreatCalculator.h"
#include "TransitionCostMatrix.h"
#include "UtilityCurves.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/BaseEnemy.h"
#include "Enemy/AdaptiveEnemy.h"
#include "Player/DepthrunCharacter.h"

UAdaptiveBehaviorComponent::UAdaptiveBehaviorComponent() {
  PrimaryComponentTick.bCanEverTick = false; // timer-driven, not tick-driven
}

void UAdaptiveBehaviorComponent::BeginPlay() {
  Super::BeginPlay();
  InitializeSubsystems();

  if (Config) {
    GetWorld()->GetTimerManager().SetTimer(
        EvaluationTimerHandle, this,
        &UAdaptiveBehaviorComponent::EvaluationTick, Config->EvaluationInterval,
        true);
  }
}

void UAdaptiveBehaviorComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason) {
  GetWorld()->GetTimerManager().ClearTimer(EvaluationTimerHandle);
  Super::EndPlay(EndPlayReason);
}

void UAdaptiveBehaviorComponent::InitializeSubsystems() {
  ContextEval = NewObject<UContextEvaluator>(this);
  ThreatCalc = NewObject<UThreatCalculator>(this);
  Resolver = NewObject<UStateTransitionResolver>(this);
  Memory = NewObject<UAdaptiveMemory>(this);
  PatternRecog = NewObject<UPatternRecognizer>(this);
  WeightManager = NewObject<UDynamicWeightManager>(this);
  UtilCurves = NewObject<UUtilityCurves>(this);
  CostMatrix = NewObject<UTransitionCostMatrix>(this);

  FSMComp = GetOwner()->FindComponentByClass<UFSMComponent>();

  if (WeightManager && Config) {
    WeightManager->ResetToDefaults(Config);
  }
  if (CostMatrix && Config) {
    CostMatrix->InitializeFromConfig(Config);
  }

  UE_LOG(LogAdaptiveBehavior, Log,
         TEXT("[AdaptiveBehavior] Subsystems initialized for %s"),
         *GetOwner()->GetName());
}

void UAdaptiveBehaviorComponent::EvaluationTick() {
  if (!Config || !ContextEval || !ThreatCalc || !Resolver || !FSMComp) return;

  ABaseEnemy* Owner = Cast<ABaseEnemy>(GetOwner());
  ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
  if (!Owner || !Player) return;

  // ── Layer 1: Context Evaluation (Stage 6C)
  LastContext = ContextEval->EvaluateContextWithMemory(Owner, Player, Memory, Config);

  // ── Layer 2: Threat Calculation (Stage 6F)
  LastThreatAssessment = ThreatCalc->CalculateThreat(LastContext, Memory, WeightManager, Config);
  OnThreatEvaluated.Broadcast(LastThreatAssessment);

  if (!bAdaptiveEnabled)
  {
      UE_LOG(LogAdaptiveBehavior, Verbose, TEXT("[AdaptiveBehavior] Bypass Layer 3 (bAdaptiveEnabled=false)"));
      return;
  }

  // ── Layer 3: Decision Making (Stage 6J)
  
  // Stage 7.1 / Diploma Hack: Convert Enum to Bravery Bias
  float BraveryModifier = 0.f;
  switch (BraveryLevel)
  {
      case EEnemyBravery::Coward: BraveryModifier = -0.3f; break; // Scared earlier
      case EEnemyBravery::Normal: BraveryModifier = 0.f; break;
      case EEnemyBravery::Brave:  BraveryModifier = 0.25f; break; // Scared later
      case EEnemyBravery::Heroic: BraveryModifier = 0.5f; break;  // Almost never scared
  }

  FThreatAssessment BiasedThreat = LastThreatAssessment;
  BiasedThreat.ThreatFinal = FMath::Clamp(BiasedThreat.ThreatFinal - BraveryModifier, 0.f, 1.f);

  float TimeInState = FSMComp->GetTimeInCurrentState();
  EFSMStateType NewState = Resolver->ResolveNextState(
      FSMComp->GetCurrentStateType(),
      BiasedThreat,
      LastContext,
      TimeInState,
      UtilCurves,
      CostMatrix,
      PatternRecog,
      Config,
      LastStateScores
  );

  // Apply decision
  EFSMStateType OldState = FSMComp->GetCurrentStateType();
  if (NewState != OldState)
  {
      FSMComp->TransitionTo(NewState);
      OnAdaptiveDecisionMade.Broadcast(OldState, NewState);
  }

  // Stage 7.5: Dynamic Ranged Mode Toggle (Auto-Ranged)
  if (Owner)
  {
      // Switch to range if Flanking (tactical) or if Chasing but far away
      const float Dist = LastContext.DistanceToPlayer;
      bool bShouldBeRanged = (NewState == EFSMStateType::Flank);
      
      // Determine distance threshold based on combat style
      float RangeThreshold = 80.f;
      switch (CombatStyle)
      {
          case EEnemyCombatStyle::MeleeOriented:  RangeThreshold = 250.f; break; // Stays melee longer
          case EEnemyCombatStyle::Balanced:       RangeThreshold = 80.f;  break;
          case EEnemyCombatStyle::RangedOriented: RangeThreshold = 30.f;  break; // Takes bow earlier
      }

      // Also stay ranged if we are far and in Attack/Chase
      if (Dist > RangeThreshold && (NewState == EFSMStateType::Attack || NewState == EFSMStateType::Chase))
      {
          bShouldBeRanged = true;
      }

      // Cast to AdaptiveEnemy to access the property
      if (AAdaptiveEnemy* AdaptiveOwner = Cast<AAdaptiveEnemy>(Owner))
      {
          AdaptiveOwner->bIsRangedMode = bShouldBeRanged;
      }
  }

  // Broadcast pattern for debug
  FString Pattern = PatternRecog->GetDominantPattern();
  if (!Pattern.IsEmpty())
  {
      OnPatternRecognized.Broadcast(Pattern);
  }

  // Stage 6M: Detailed Logging
  if (UE_LOG_ACTIVE(LogAdaptiveBehavior, Verbose))
  {
      FString ScoresLog;
      for (const FStateScore& S : LastStateScores)
      {
          ScoresLog += FString::Printf(TEXT("%s=%.2f "), *UFSMComponent::GetStateName(S.State), S.FinalScore);
      }

      UE_LOG(LogAdaptiveBehavior, Verbose,
          TEXT("[AdaptiveBehavior] Enemy=%s | T=%.3f C=%.3f | %s | → %s"),
          *Owner->GetName(),
          LastThreatAssessment.ThreatFinal,
          LastThreatAssessment.Confidence,
          *ScoresLog,
          *UFSMComponent::GetStateName(NewState));
  }
}

void UAdaptiveBehaviorComponent::OnDamageDealt() {
  if (WeightManager && Config)
  {
      // Reward = +1 (success)
      WeightManager->UpdateWeights(1.0f, LastContext, Config);
  }
}

void UAdaptiveBehaviorComponent::OnDamageTaken() {
  if (WeightManager && Config)
  {
      // Reward = -1 (failure)
      WeightManager->UpdateWeights(-1.0f, LastContext, Config);
  }
}

void UAdaptiveBehaviorComponent::HandlePlayerAction(
    const FPlayerActionEvent &ActionEvent) {
  if (Memory) {
    FMemoryEvent Ev;
    Ev.ActionType = ActionEvent.ActionType;
    Ev.Timestamp = ActionEvent.Timestamp;
    Ev.Location = ActionEvent.Location;
    Ev.Intensity = ActionEvent.Intensity;
    Memory->RecordEvent(Ev);
  }
  if (PatternRecog) {
    PatternRecog->AddAction(ActionEvent.ActionType);
  }
}

float UAdaptiveBehaviorComponent::GetThreatFinal() const {
  return LastThreatAssessment.ThreatFinal;
}
float UAdaptiveBehaviorComponent::GetConfidence() const {
  return LastThreatAssessment.Confidence;
}
TArray<float> UAdaptiveBehaviorComponent::GetCurrentWeights() const {
  return WeightManager ? WeightManager->GetWeights() : TArray<float>();
}
FString UAdaptiveBehaviorComponent::GetRecognizedPattern() const {
  return PatternRecog ? PatternRecog->GetDominantPattern() : TEXT("");
}
TArray<FStateScore> UAdaptiveBehaviorComponent::GetLastStateScores() const {
  return LastStateScores;
}
FThreatAssessment UAdaptiveBehaviorComponent::GetLastThreatAssessment() const {
  return LastThreatAssessment;
}
