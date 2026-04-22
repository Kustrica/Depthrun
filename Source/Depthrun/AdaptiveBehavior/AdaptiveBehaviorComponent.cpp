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

#include "Kismet/GameplayStatics.h"
#include "Enemy/BaseEnemy.h"
#include "Player/DepthrunCharacter.h"

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

  // ── Layer 3: Decision Making (Stage 6J)
  float TimeInState = FSMComp->GetTimeInCurrentState();
  EFSMStateType NewState = Resolver->ResolveNextState(
      FSMComp->GetCurrentStateType(),
      LastThreatAssessment,
      LastContext,
      TimeInState,
      UtilCurves,
      CostMatrix,
      PatternRecog,
      Config,
      LastStateScores
  );

  // Apply decision
  if (NewState != FSMComp->GetCurrentStateType())
  {
      FSMComp->TransitionTo(NewState);
      OnAdaptiveDecisionMade.Broadcast(NewState);
  }

  // Broadcast pattern for debug
  FString Pattern = PatternRecog->GetDominantPattern();
  if (!Pattern.IsEmpty())
  {
      OnPatternRecognized.Broadcast(Pattern);
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
