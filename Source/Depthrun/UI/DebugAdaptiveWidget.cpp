// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DebugAdaptiveWidget.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

void UDebugAdaptiveWidget::BindToComponent(UAdaptiveBehaviorComponent* Component)
{
	BoundComponent = Component;
}

void UDebugAdaptiveWidget::RefreshDisplay()
{
	if (!BoundComponent) return;
	OnDisplayRefreshed();
}

float UDebugAdaptiveWidget::GetThreatFinal()   const { return BoundComponent ? BoundComponent->GetThreatFinal()  : 0.f; }
float UDebugAdaptiveWidget::GetConfidence()     const { return BoundComponent ? BoundComponent->GetConfidence()   : 0.f; }
TArray<float> UDebugAdaptiveWidget::GetWeights() const { return BoundComponent ? BoundComponent->GetCurrentWeights() : TArray<float>(); }
FString UDebugAdaptiveWidget::GetPatternString() const { return BoundComponent ? BoundComponent->GetRecognizedPattern() : TEXT("—"); }
TArray<FStateScore> UDebugAdaptiveWidget::GetStateScores() const { return BoundComponent ? BoundComponent->GetLastStateScores() : TArray<FStateScore>(); }

FText UDebugAdaptiveWidget::GetGodModeText() const
{
    if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
    {
        return Player->GetGodStatusText();
    }
    return FText::GetEmpty();
}

FText UDebugAdaptiveWidget::GetSuperAttackText() const
{
    if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
    {
        return Player->GetSuperAttackStatusText();
    }
    return FText::GetEmpty();
}
