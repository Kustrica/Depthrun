// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DemonstrationSubsystem.h"
#include "DepthrunLogChannels.h"

void UDemonstrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAdaptiveBehavior, Log, TEXT("[DiagLog] DemonstrationSubsystem initialized"));
}

void UDemonstrationSubsystem::Deinitialize()
{
	bVerboseLogging = false;
	Super::Deinitialize();
}

void UDemonstrationSubsystem::LogEvaluationResult(float ThreatValue,
	const TArray<float>& Weights, const FString& ChosenState, const FString& Pattern)
{
	if (!bVerboseLogging) return;

	FString WeightStr;
	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		WeightStr += FString::Printf(TEXT("w%d=%.3f"), i, Weights[i]);
		if (i < Weights.Num() - 1) WeightStr += TEXT(" ");
	}

	UE_LOG(LogAdaptiveBehavior, Log,
		TEXT("[DiagLog] Eval | Threat=%.3f | %s | State=%s | Pattern=%s"),
		ThreatValue, *WeightStr, *ChosenState, *Pattern);
}

void UDemonstrationSubsystem::LogCombatEvent(EDemoCombatEvent EventType,
	const FString& Instigator, const FString& Target)
{
	if (!bVerboseLogging) return;

	const TCHAR* EventName = TEXT("Unknown");
	switch (EventType)
	{
		case EDemoCombatEvent::PlayerShot:     EventName = TEXT("PlayerShot");     break;
		case EDemoCombatEvent::PlayerMelee:    EventName = TEXT("PlayerMelee");    break;
		case EDemoCombatEvent::EnemyShot:      EventName = TEXT("EnemyShot");      break;
		case EDemoCombatEvent::EnemyMelee:     EventName = TEXT("EnemyMelee");     break;
		case EDemoCombatEvent::DamageReceived: EventName = TEXT("DamageReceived"); break;
	}

	UE_LOG(LogAdaptiveBehavior, Log,
		TEXT("[DiagLog] Combat | %s | %s -> %s"),
		EventName, *Instigator, *Target);
}

void UDemonstrationSubsystem::LogWeightUpdate(const TArray<float>& OldWeights,
	const TArray<float>& NewWeights, float Reward)
{
	if (!bVerboseLogging) return;

	FString Delta;
	for (int32 i = 0; i < 6; ++i)
	{
		float Old = OldWeights.IsValidIndex(i) ? OldWeights[i] : 0.f;
		float New = NewWeights.IsValidIndex(i) ? NewWeights[i] : 0.f;
		Delta += FString::Printf(TEXT("w%d: %.3f->%.3f"), i, Old, New);
		if (i < 5) Delta += TEXT(" | ");
	}

	UE_LOG(LogAdaptiveBehavior, Log,
		TEXT("[DiagLog] Weights | Reward=%.2f | %s"),
		Reward, *Delta);
}

void UDemonstrationSubsystem::LogStateTransition(const FString& FromState,
	const FString& ToState, float ThreatValue)
{
	if (!bVerboseLogging) return;

	UE_LOG(LogAdaptiveBehavior, Log,
		TEXT("[DiagLog] Transition | %s -> %s | Threat=%.3f"),
		*FromState, *ToState, ThreatValue);
}
