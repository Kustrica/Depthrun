// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DebugAdaptiveWidget.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Enemy/AdaptiveEnemy.h"
#include "FSM/FSMComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Pre-size arrays so GetEnemySnapshot never goes out of bounds.
	// Do NOT scan or start the timer here — the widget starts hidden
	// and scanning would assign #N labels above enemies prematurely.
	// HUD calls Activate() when the user presses F3.
	Snapshots.SetNum(MaxTrackedEnemies);
	TrackedEnemies.SetNum(MaxTrackedEnemies);
}

void UDebugAdaptiveWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}
	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────────────
// RefreshDisplay — called from Blueprint Event Tick
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::RefreshDisplay()
{
	// ScanForEnemies is now on its own timer (ScanTimerHandle) — no manual
	// DeltaTime accumulation needed. The old code used GetDeltaSeconds()
	// (~0.016s per frame) in a 0.2s timer callback, so a 2s ScanInterval
	// actually took ~25s to fire. The separate timer fixes this.

	// Rebuild snapshots for all tracked slots and push them into bound UMG widgets.
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
		ApplyColumn(i, Snapshots[i]);
	}

	OnDisplayRefreshed();
}

// ─────────────────────────────────────────────────────────────────────────────
// Activate / Deactivate — called by HUD when F3 toggles visibility
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::Activate()
{
	if (bActive) return;
	bActive = true;

	if (UWorld* World = GetWorld())
	{
		// Display refresh (~5 Hz) — updates progress bars and text.
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle, this, &UDebugAdaptiveWidget::RefreshDisplay, 0.2f, true);

		// Enemy re-scan (every ScanInterval seconds) — picks up new/dead enemies
		// when the player enters a new room. Separate timer avoids the old bug
		// where GetDeltaSeconds() in a 0.2s callback took ~25s to reach 2.0.
		World->GetTimerManager().SetTimer(
			ScanTimerHandle, this, &UDebugAdaptiveWidget::ScanForEnemies, ScanInterval, true);
	}

	// Immediate first scan so #N labels appear at once.
	ForceRefresh();
}

void UDebugAdaptiveWidget::Deactivate()
{
	if (!bActive) return;
	bActive = false;

	// Stop both timers — no polling or scanning while hidden.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// ForceRefresh — bypasses ScanInterval, used by HUD on widget open (F3)
// ─────────────────────────────────────────────────────────────────────────────

void UDebugAdaptiveWidget::ForceRefresh()
{
	ScanForEnemies();
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
	// Use the BP class name (e.g. "BP_AdaptiveEnemy") instead of the instance
	// name. GetName() returns a unique object name whose numeric suffix is tiny
	// in PIE (_0, _1) but huge in Standalone/Packaged builds (_2147481068),
	// because FName indices are global and accumulate before level load.
	{
		FString ClassName = Enemy->GetClass()->GetName();
		ClassName.RemoveFromEnd(TEXT("_C"));
		S.EnemyName = MoveTemp(ClassName);
	}
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
		S.AllyCountNorm        = Ctx.AllyCountNorm;
		S.RoomDensityNorm      = Ctx.RoomDensityNorm;
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
// ApplyColumn — push snapshot data into bound UMG widgets
// All bindings are Optional: widgets the user didn't add are silently skipped.
// User just needs to name UMG elements txt_state_0, pb_threat_0, etc.
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	void SetTextSafe(UTextBlock* T, const FString& S)
	{
		if (!T) return;
		T->SetText(FText::FromString(S));
		// Force center justification so the text is centered within the widget
		// bounds. SetText() does not reset Justification, but we re-apply it
		// each tick to make sure it takes effect regardless of editor defaults.
		// This is cheap (just a property set) and ensures consistency.
		T->SetJustification(ETextJustify::Center);
		// Note: We do NOT force HAlign_Fill here because it requires slot
		// casting and is expensive. The user should set HAlign_Fill in the
		// editor for the parent slot (VerticalBox/HorizontalBox) for
		// justification to be visible.
	}
	void SetPercentSafe(UProgressBar* P, float V)
	{
		if (P) P->SetPercent(FMath::Clamp(V, 0.f, 1.f));
	}
}

FString UDebugAdaptiveWidget::FormatWeights(const TArray<float>& W)
{
	if (W.Num() < 6) return TEXT("Weights: —");
	// Two lines with "w:" prefix so the user can distinguish WEIGHTS
	// (slow-learning multipliers) from FACTORS (real-time context bars).
	// "Wpn" instead of "Threat" to avoid confusion with ThreatFinal bar.
	return FString::Printf(
		TEXT("w: Dist %.2f  Wpn %.2f\nw: HP %.2f  Allies %.2f\nw: Dens %.2f  Mem %.2f"),
		W[0], W[1], W[2], W[3], W[4], W[5]);
}

void UDebugAdaptiveWidget::ApplyColumn(int32 Index, const FEnemyDebugSnapshot& S)
{
	UWidget*      ColRoot   = nullptr;
	UTextBlock*   THeader   = nullptr;
	UTextBlock*   TState    = nullptr;
	UTextBlock*   TMode     = nullptr;
	UTextBlock*   TPattern  = nullptr;
	UTextBlock*   TBravery  = nullptr;
	UTextBlock*   TWeights  = nullptr;
	UTextBlock*   TDist     = nullptr;
	UProgressBar* PThreat   = nullptr;
	UProgressBar* PDist     = nullptr;
	UProgressBar* PHP       = nullptr;
	UProgressBar* PMem      = nullptr;
	UProgressBar* PWpn      = nullptr;
	UProgressBar* PAllies   = nullptr;
	UProgressBar* PDens     = nullptr;
	UTextBlock*   TThreatV  = nullptr;
	UTextBlock*   TDistV    = nullptr;
	UTextBlock*   THPV      = nullptr;
	UTextBlock*   TMemV     = nullptr;
	UTextBlock*   TWpnV     = nullptr;
	UTextBlock*   TAlliesV  = nullptr;
	UTextBlock*   TDensV    = nullptr;

	switch (Index)
	{
		case 0: ColRoot=Column_0; THeader=txt_header_0; TState=txt_state_0; TMode=txt_mode_0;
		        TPattern=txt_pattern_0; TBravery=txt_bravery_0; TWeights=txt_weights_0; TDist=txt_dist_0;
		        PThreat=pb_threat_0; PDist=pb_dist_0; PHP=pb_hp_0; PMem=pb_mem_0;
		        PWpn=pb_wpn_0; PAllies=pb_allies_0; PDens=pb_dens_0;
		        TThreatV=txt_threat_val_0; TDistV=txt_distnorm_val_0; THPV=txt_hp_val_0; TMemV=txt_mem_val_0;
		        TWpnV=txt_wpn_val_0; TAlliesV=txt_allies_val_0; TDensV=txt_dens_val_0; break;
		case 1: ColRoot=Column_1; THeader=txt_header_1; TState=txt_state_1; TMode=txt_mode_1;
		        TPattern=txt_pattern_1; TBravery=txt_bravery_1; TWeights=txt_weights_1; TDist=txt_dist_1;
		        PThreat=pb_threat_1; PDist=pb_dist_1; PHP=pb_hp_1; PMem=pb_mem_1;
		        PWpn=pb_wpn_1; PAllies=pb_allies_1; PDens=pb_dens_1;
		        TThreatV=txt_threat_val_1; TDistV=txt_distnorm_val_1; THPV=txt_hp_val_1; TMemV=txt_mem_val_1;
		        TWpnV=txt_wpn_val_1; TAlliesV=txt_allies_val_1; TDensV=txt_dens_val_1; break;
		case 2: ColRoot=Column_2; THeader=txt_header_2; TState=txt_state_2; TMode=txt_mode_2;
		        TPattern=txt_pattern_2; TBravery=txt_bravery_2; TWeights=txt_weights_2; TDist=txt_dist_2;
		        PThreat=pb_threat_2; PDist=pb_dist_2; PHP=pb_hp_2; PMem=pb_mem_2;
		        PWpn=pb_wpn_2; PAllies=pb_allies_2; PDens=pb_dens_2;
		        TThreatV=txt_threat_val_2; TDistV=txt_distnorm_val_2; THPV=txt_hp_val_2; TMemV=txt_mem_val_2;
		        TWpnV=txt_wpn_val_2; TAlliesV=txt_allies_val_2; TDensV=txt_dens_val_2; break;
		case 3: ColRoot=Column_3; THeader=txt_header_3; TState=txt_state_3; TMode=txt_mode_3;
		        TPattern=txt_pattern_3; TBravery=txt_bravery_3; TWeights=txt_weights_3; TDist=txt_dist_3;
		        PThreat=pb_threat_3; PDist=pb_dist_3; PHP=pb_hp_3; PMem=pb_mem_3;
		        PWpn=pb_wpn_3; PAllies=pb_allies_3; PDens=pb_dens_3;
		        TThreatV=txt_threat_val_3; TDistV=txt_distnorm_val_3; THPV=txt_hp_val_3; TMemV=txt_mem_val_3;
		        TWpnV=txt_wpn_val_3; TAlliesV=txt_allies_val_3; TDensV=txt_dens_val_3; break;
		default: return;
	}

	// Hide entire column when no enemy in this slot.
	if (ColRoot)
	{
		ColRoot->SetVisibility(S.bValid
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
	if (!S.bValid) return;

	SetTextSafe(THeader,  FString::Printf(TEXT("#%d — %s"), S.EnemyIndex + 1, *S.EnemyName));
	SetTextSafe(TState,   FString::Printf(TEXT("State: %s"), *S.CurrentFSMState));
	SetTextSafe(TMode,    S.bRangedMode ? TEXT("Mode: Ranged") : TEXT("Mode: Melee"));
	SetTextSafe(TPattern, FString::Printf(TEXT("Pattern: %s"),
		S.RecognizedPattern.IsEmpty() ? TEXT("None") : *S.RecognizedPattern));
	SetTextSafe(TBravery, FString::Printf(TEXT("%s / %s"), *S.BraveryLabel, *S.CombatStyleLabel));
	SetTextSafe(TWeights, FormatWeights(S.Weights));
	SetTextSafe(TDist,    FString::Printf(TEXT("Dist: %.0f uu"), S.DistanceRaw));

	SetPercentSafe(PThreat, S.ThreatFinal);
	SetPercentSafe(PDist,   S.DistanceNorm);
	SetPercentSafe(PHP,     S.EnemyHPNorm);
	SetPercentSafe(PMem,    S.MemoryAggressiveness);
	SetPercentSafe(PWpn,    S.WeaponThreatNorm);
	SetPercentSafe(PAllies, S.AllyCountNorm);
	SetPercentSafe(PDens,   S.RoomDensityNorm);

	// Per-bar inline labels (Optional widgets — user adds them if desired).
	// Format keeps it self-explanatory: "Name: 0.XX" so the committee sees
	// both the metric name and its current normalized value.
	SetTextSafe(TThreatV,  FString::Printf(TEXT("Threat: %.2f"),    S.ThreatFinal));
	SetTextSafe(TDistV,    FString::Printf(TEXT("Dist Norm: %.2f"), S.DistanceNorm));
	SetTextSafe(THPV,      FString::Printf(TEXT("Enemy HP: %.2f"),  S.EnemyHPNorm));
	SetTextSafe(TMemV,     FString::Printf(TEXT("Memory: %.2f"),    S.MemoryAggressiveness));
	SetTextSafe(TWpnV,     FString::Printf(TEXT("Weapon: %.2f"),    S.WeaponThreatNorm));
	SetTextSafe(TAlliesV,  FString::Printf(TEXT("Allies: %.2f"),    S.AllyCountNorm));
	SetTextSafe(TDensV,    FString::Printf(TEXT("Density: %.2f"),   S.RoomDensityNorm));
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
