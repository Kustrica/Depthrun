// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DebugAdaptiveWidget.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Enemy/AdaptiveEnemy.h"
#include "FSM/FSMComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Pre-size arrays so GetEnemySnapshot never goes out of bounds
	Snapshots.SetNum(MaxTrackedEnemies);
	TrackedEnemies.SetNum(MaxTrackedEnemies);

	ScanForEnemies();
}

// ─────────────────────────────────────────────────────────────────────────────
// RefreshDisplay — called from Blueprint Event Tick
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::RefreshDisplay()
{
	// Periodically re-scan for new/dead enemies
	float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	TimeSinceLastScan += DeltaTime;
	if (TimeSinceLastScan >= ScanInterval)
	{
		ScanForEnemies();
		TimeSinceLastScan = 0.f;
	}

	// Rebuild snapshots for all tracked slots
	Snapshots.SetNum(MaxTrackedEnemies);
	for (int32 i = 0; i < MaxTrackedEnemies; ++i)
	{
		AAdaptiveEnemy* Enemy = (i < TrackedEnemies.Num()) ? TrackedEnemies[i].Get() : nullptr;
		if (IsValid(Enemy))
		{
			Snapshots[i] = BuildSnapshot(Enemy);
		}
		else
		{
			Snapshots[i] = FEnemyDebugSnapshot(); // empty slot
		}
	}

	OnDisplayRefreshed();
}

// ─────────────────────────────────────────────────────────────────────────────
// ForceRefresh — bypasses ScanInterval, used by HUD on widget open (F3)
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::ForceRefresh()
{
	// Reset timer so the next regular RefreshDisplay() also starts fresh
	TimeSinceLastScan = ScanInterval;
	RefreshDisplay();
}

// ─────────────────────────────────────────────────────────────────────────────
// ScanForEnemies — find up to MaxTrackedEnemies live AAdaptiveEnemy actors
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::ScanForEnemies()
{
	TrackedEnemies.Reset();
	TrackedEnemies.SetNum(MaxTrackedEnemies);

	if (!GetWorld()) return;

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAdaptiveEnemy::StaticClass(), Found);

	// Sort by actor name for stable column order across re-scans
	Found.Sort([](const AActor& A, const AActor& B)
	{
		return A.GetName() < B.GetName();
	});

	int32 SlotIndex = 0;
	for (AActor* Actor : Found)
	{
		if (SlotIndex >= MaxTrackedEnemies) break;
		if (AAdaptiveEnemy* Enemy = Cast<AAdaptiveEnemy>(Actor))
		{
			if (IsValid(Enemy))
			{
				// Assign slot index and show #N label above the enemy
				Enemy->AssignDebugIndex(SlotIndex);
				TrackedEnemies[SlotIndex++] = Enemy;
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildSnapshot — collect all debug data from one enemy
// ─────────────────────────────────────────────────────────────────────────────

FEnemyDebugSnapshot UDebugAdaptiveWidget::BuildSnapshot(AAdaptiveEnemy* Enemy) const
{
	FEnemyDebugSnapshot S;
	if (!IsValid(Enemy)) return S;

	S.bValid      = true;
	S.EnemyName   = Enemy->GetName();
	S.EnemyIndex  = Enemy->EnemyDebugIndex;
	S.bRangedMode = Enemy->bIsRangedMode;

	// ── AdaptiveBehaviorComponent data ──────────────────────────────────────
	UAdaptiveBehaviorComponent* AC = Enemy->FindComponentByClass<UAdaptiveBehaviorComponent>();
	if (AC)
	{
		S.ThreatFinal       = AC->GetThreatFinal();
		S.Confidence        = AC->GetConfidence();
		S.Weights           = AC->GetCurrentWeights();
		S.StateScores       = AC->GetLastStateScores();
		S.RecognizedPattern = AC->GetRecognizedPattern();

		// Personality labels
		S.BraveryLabel     = BraveryToString(AC->BraveryLevel);
		S.CombatStyleLabel = CombatStyleToString(AC->CombatStyle);

		// Raw context factors for the "what feeds T_final" section
		FContextData Ctx = AC->GetLastContext();
		S.DistanceNorm         = Ctx.DistanceNorm;
		S.EnemyHPNorm          = Ctx.EnemyHPRatioNorm;
		S.WeaponThreatNorm     = Ctx.WeaponThreatNorm;
		S.MemoryAggressiveness = Ctx.MemoryAggressiveness;
	}

	// ── FSM state ────────────────────────────────────────────────────────────
	UFSMComponent* FSMComp = Enemy->FindComponentByClass<UFSMComponent>();
	if (FSMComp)
	{
		S.CurrentFSMState = UFSMComponent::GetStateName(FSMComp->GetCurrentStateType());
	}

	// ── Distance to player ───────────────────────────────────────────────────
	if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(
	        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		S.DistanceRaw = FVector::Dist2D(Enemy->GetActorLocation(), Player->GetActorLocation());
	}

	return S;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public accessors
// ─────────────────────────────────────────────────────────────────────────────

FEnemyDebugSnapshot UDebugAdaptiveWidget::GetEnemySnapshot(int32 Index) const
{
	if (Snapshots.IsValidIndex(Index)) return Snapshots[Index];
	return FEnemyDebugSnapshot();
}

int32 UDebugAdaptiveWidget::GetTrackedEnemyCount() const
{
	int32 Count = 0;
	for (const FEnemyDebugSnapshot& S : Snapshots)
	{
		if (S.bValid) ++Count;
	}
	return Count;
}

// ─────────────────────────────────────────────────────────────────────────────
// Legacy single-enemy API
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::BindToComponent(UAdaptiveBehaviorComponent* Component)
{
	BoundComponent = Component;
}

float UDebugAdaptiveWidget::GetThreatFinal()     const { return BoundComponent ? BoundComponent->GetThreatFinal()        : 0.f; }
float UDebugAdaptiveWidget::GetConfidence()       const { return BoundComponent ? BoundComponent->GetConfidence()         : 0.f; }
TArray<float> UDebugAdaptiveWidget::GetWeights()  const { return BoundComponent ? BoundComponent->GetCurrentWeights()     : TArray<float>(); }
FString UDebugAdaptiveWidget::GetPatternString()  const { return BoundComponent ? BoundComponent->GetRecognizedPattern()  : TEXT("—"); }
TArray<FStateScore> UDebugAdaptiveWidget::GetStateScores() const
{
	return BoundComponent ? BoundComponent->GetLastStateScores() : TArray<FStateScore>();
}

FText UDebugAdaptiveWidget::GetGodModeText() const
{
	if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(
	        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		return Player->GetGodStatusText();
	}
	return FText::GetEmpty();
}

FText UDebugAdaptiveWidget::GetSuperAttackText() const
{
	if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(
	        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		return Player->GetSuperAttackStatusText();
	}
	return FText::GetEmpty();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

FString UDebugAdaptiveWidget::BraveryToString(EEnemyBravery Bravery)
{
	switch (Bravery)
	{
		case EEnemyBravery::Coward:  return TEXT("Coward");
		case EEnemyBravery::Normal:  return TEXT("Normal");
		case EEnemyBravery::Brave:   return TEXT("Brave");
		case EEnemyBravery::Heroic:  return TEXT("Heroic");
		default:                     return TEXT("?");
	}
}

FString UDebugAdaptiveWidget::CombatStyleToString(EEnemyCombatStyle Style)
{
	switch (Style)
	{
		case EEnemyCombatStyle::MeleeOriented:  return TEXT("Melee");
		case EEnemyCombatStyle::Balanced:       return TEXT("Balanced");
		case EEnemyCombatStyle::RangedOriented: return TEXT("Ranged");
		default:                                return TEXT("?");
	}
}
