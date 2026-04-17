// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "AdaptiveBehaviorComponent.h"
#include "AdaptiveConfig.h"
#include "ContextEvaluator.h"
#include "ThreatCalculator.h"
#include "StateTransitionResolver.h"
#include "AdaptiveMemory.h"
#include "PatternRecognizer.h"
#include "DynamicWeightManager.h"
#include "UtilityCurves.h"
#include "TransitionCostMatrix.h"
#include "FSM/FSMComponent.h"
#include "DepthrunLogChannels.h"

UAdaptiveBehaviorComponent::UAdaptiveBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // timer-driven, not tick-driven
}

void UAdaptiveBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSubsystems();

	if (Config)
	{
		GetWorld()->GetTimerManager().SetTimer(
			EvaluationTimerHandle,
			this,
			&UAdaptiveBehaviorComponent::EvaluationTick,
			Config->EvaluationInterval,
			true);
	}
}

void UAdaptiveBehaviorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(EvaluationTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void UAdaptiveBehaviorComponent::InitializeSubsystems()
{
	ContextEval   = NewObject<UContextEvaluator>(this);
	ThreatCalc    = NewObject<UThreatCalculator>(this);
	Resolver      = NewObject<UStateTransitionResolver>(this);
	Memory        = NewObject<UAdaptiveMemory>(this);
	PatternRecog  = NewObject<UPatternRecognizer>(this);
	WeightManager = NewObject<UDynamicWeightManager>(this);
	UtilCurves    = NewObject<UUtilityCurves>(this);
	CostMatrix    = NewObject<UTransitionCostMatrix>(this);

	FSMComp = GetOwner()->FindComponentByClass<UFSMComponent>();

	if (WeightManager && Config) { WeightManager->ResetToDefaults(Config); }
	if (CostMatrix    && Config) { CostMatrix->InitializeFromConfig(Config); }

	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[AdaptiveBehavior] Subsystems initialized for %s"), *GetOwner()->GetName());
}

void UAdaptiveBehaviorComponent::EvaluationTick()
{
	// TODO (Stage 6L): full pipeline implementation
	// Context → Threat → Resolve → Transition
	UE_LOG(LogAdaptiveBehavior, Verbose, TEXT("[AdaptiveBehavior] EvaluationTick — stub"));
}

void UAdaptiveBehaviorComponent::OnDamageDealt()
{
	// TODO (Stage 6L): WeightManager->UpdateWeights(+1.f, ...)
}

void UAdaptiveBehaviorComponent::OnDamageTaken()
{
	// TODO (Stage 6L): WeightManager->UpdateWeights(-1.f, ...)
}

void UAdaptiveBehaviorComponent::HandlePlayerAction(const FPlayerActionEvent& ActionEvent)
{
	if (Memory)
	{
		FMemoryEvent Ev;
		Ev.ActionType = ActionEvent.ActionType;
		Ev.Timestamp  = ActionEvent.Timestamp;
		Ev.Location   = ActionEvent.Location;
		Ev.Intensity  = ActionEvent.Intensity;
		Memory->RecordEvent(Ev);
	}
	if (PatternRecog) { PatternRecog->AddAction(ActionEvent.ActionType); }
}

float UAdaptiveBehaviorComponent::GetThreatFinal()        const { return LastThreatAssessment.ThreatFinal; }
float UAdaptiveBehaviorComponent::GetConfidence()          const { return LastThreatAssessment.Confidence; }
TArray<float> UAdaptiveBehaviorComponent::GetCurrentWeights() const { return WeightManager ? WeightManager->GetWeights() : TArray<float>(); }
FString UAdaptiveBehaviorComponent::GetRecognizedPattern() const { return PatternRecog ? PatternRecog->GetDominantPattern() : TEXT(""); }
TArray<FStateScore> UAdaptiveBehaviorComponent::GetLastStateScores() const { return LastStateScores; }
FThreatAssessment UAdaptiveBehaviorComponent::GetLastThreatAssessment() const { return LastThreatAssessment; }
