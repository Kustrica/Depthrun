// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Flank.h"
#include "Enemy/BaseEnemy.h"
#include "FSM/FSMComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Player/DepthrunCharacter.h"

// How long (seconds) enemy stays in Flank before re-evaluating.
static constexpr float kFlankMaxDuration = 3.5f;

void UFSMState_Flank::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);

	// Randomise left/right flank side.
	FlankSide               = (FMath::RandBool()) ? 1 : -1;
	TimeSinceDirectionRefresh = 0.f;

	RecalculateFlankDirection(Owner);

	UE_LOG(LogFSM, Log, TEXT("[Flank] Enter — %s  side=%d"), *GetNameSafe(Owner), FlankSide);
}

void UFSMState_Flank::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	if (!Owner) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player)) return;

	// Commercial Fix: stop flanking if player is dead
	if (ADepthrunCharacter* DepthrunPlayer = Cast<ADepthrunCharacter>(Player))
	{
		if (DepthrunPlayer->IsDead())
		{
			if (UFSMComponent* FSMComp = Owner->GetComponentByClass<UFSMComponent>())
			{
				FSMComp->TransitionTo(EFSMStateType::Idle);
			}
			return;
		}
	}

	const float Dist = FVector::Dist2D(Owner->GetActorLocation(), Player->GetActorLocation());

	UFSMComponent* FSM = Owner->GetComponentByClass<UFSMComponent>();
	if (!FSM) return;

	// ── Exit conditions ────────────────────────────────────────────────────
	if (TimeInState >= kFlankMaxDuration)
	{
		// After enough flanking, re-evaluate from Idle.
		FSM->TransitionTo(EFSMStateType::Idle);
		return;
	}
	if (Dist <= Owner->AttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Attack);
		return;
	}

	// ── Refresh direction every FlankDirectionRefreshInterval ─────────────
	TimeSinceDirectionRefresh += DeltaTime;
	if (TimeSinceDirectionRefresh >= FlankDirectionRefreshInterval)
	{
		TimeSinceDirectionRefresh = 0.f;
		RecalculateFlankDirection(Owner);
	}

	Owner->AddMovementInput(FlankDirection);
}

void UFSMState_Flank::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	FlankDirection = FVector::ZeroVector;
	UE_LOG(LogFSM, Verbose, TEXT("[Flank] Exit"));
}

void UFSMState_Flank::RecalculateFlankDirection(ABaseEnemy* Owner)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player) || !Owner) return;

	// Perpendicular to the player→enemy vector (rotated 90° on Z axis).
	const FVector ToPlayer   = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	const FVector Perp       = FVector(-ToPlayer.Y, ToPlayer.X, 0.f) * static_cast<float>(FlankSide); // 90° rotated
	
	// Stage 12: Shallow Angled Approach (not a wide 90° arc, but a 30-45° push)
	// We mix the direct direction to player with the perpendicular direction.
	// 0.8 / 0.5 mix gives approx 32 degrees angle.
	FlankDirection           = (ToPlayer * 0.8f + Perp * 0.5f).GetSafeNormal2D();
}
