// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Attack.h"
#include "Enemy/BaseEnemy.h"
#include "FSM/FSMComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

void UFSMState_Attack::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	TimeSinceLastAttack = AttackCooldown; // Allow first attack immediately
	bAttackCoolingDown  = false;
	TimeInAttackState   = 0.f;

	// Stop moving during attack.
	if (Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopMovementImmediately();
	}

	UE_LOG(LogFSM, Log, TEXT("[Attack] Enter — %s"), *GetNameSafe(Owner));
}

void UFSMState_Attack::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	if (!Owner) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player)) return;

	// Commercial Fix: stop attacking if player is dead
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

	TimeSinceLastAttack += DeltaTime;
	TimeInAttackState   += DeltaTime;

	// ── Exit condition: player moved out of attack range ───────────────────
	// Only allow exit after MinTimeBeforeDistanceExit to prevent frame-level
	// Chase<->Attack oscillation when the enemy sits exactly at AttackRange.
	if (TimeInAttackState >= MinTimeBeforeDistanceExit && Dist > Owner->AttackRange * 1.2f)
	{
		FSM->TransitionTo(EFSMStateType::Chase);
		return;
	}

	// ── Commercial Fix: Ranged enemies should back off if player is too close
	if (Owner->GetEnemyType() == EEnemyType::Ranged && Dist < Owner->MinAttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Retreat);
		return;
	}

	// ── Attack cooldown ────────────────────────────────────────────────────
	if (TimeSinceLastAttack >= AttackCooldown)
	{
		TimeSinceLastAttack = 0.f;
		Owner->PerformMeleeAttack(); // Delegate actual attack to the enemy actor
		UE_LOG(LogFSM, Log, TEXT("[Attack] %s → attack fired"), *GetNameSafe(Owner));
	}

    // Stage 12: Separation during attack
    FVector Separation = Owner->GetSeparationSteering();
    if (!Separation.IsNearlyZero())
    {
        Owner->AddMovementInput(Separation, 0.5f);
    }
}

void UFSMState_Attack::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[Attack] Exit"));
}
