// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Idle.h"
#include "Enemy/BaseEnemy.h"
#include "FSM/FSMComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

// Detection check interval in seconds (avoid checking every frame).
static constexpr float kIdleCheckInterval = 0.35f;

void UFSMState_Idle::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	TimeSinceLastLook = 0.f;

	// Stop all movement immediately.
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopMovementImmediately();
	}

	UE_LOG(LogFSM, Log, TEXT("[Idle] Enter — %s"), *GetNameSafe(Owner));
}

void UFSMState_Idle::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	if (!Owner) return;

	TimeSinceLastLook += DeltaTime;
	if (TimeSinceLastLook < kIdleCheckInterval) return;
	TimeSinceLastLook = 0.f;

	// ── Player detection ──────────────────────────────────────────────────
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player)) return;

	// Commercial Fix: stop detecting if player is dead
	if (ADepthrunCharacter* DepthrunPlayer = Cast<ADepthrunCharacter>(Player))
	{
		if (DepthrunPlayer->IsDead()) return;
	}

	const float Dist = FVector::Dist2D(Owner->GetActorLocation(), Player->GetActorLocation());

	UFSMComponent* FSM = Owner->GetComponentByClass<UFSMComponent>();
	if (!FSM) return;

	// Commercial Fix: Direct transition to Retreat if player is already inside MinAttackRange
	if (Owner->GetEnemyType() == EEnemyType::Ranged && Dist < Owner->MinAttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Retreat);
		return;
	}

	if (Dist <= Owner->AttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Attack);
	}
	else if (Dist <= Owner->DetectionRange)
	{
		FSM->TransitionTo(EFSMStateType::Chase);
	}
}

void UFSMState_Idle::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[Idle] Exit"));
}
