// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Chase.h"
#include "Enemy/BaseEnemy.h"
#include "FSM/FSMComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

void UFSMState_Chase::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	UE_LOG(LogFSM, Log, TEXT("[Chase] Enter — %s"), *GetNameSafe(Owner));
}

void UFSMState_Chase::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	if (!Owner) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player)) return;

	// Commercial Fix: stop chasing if player is dead
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

	const FVector OwnerLoc  = Owner->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const float   Dist      = FVector::Dist2D(OwnerLoc, PlayerLoc);

	UFSMComponent* FSM = Owner->GetComponentByClass<UFSMComponent>();
	if (!FSM) return;

	// ── Transition checks ─────────────────────────────────────────────────
	if (Dist <= Owner->AttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Attack);
		return;
	}
	if (Dist > Owner->DetectionRange * 1.5f) // lost sight (50% grace buffer)
	{
		FSM->TransitionTo(EFSMStateType::Idle);
		return;
	}

	// ── Move toward player ────────────────────────────────────────────────
	// In top-down 2D with gravity=0 and Flying/Walking mode,
	// AddMovementInput scales with MaxWalkSpeed/MaxFlySpeed.
	const FVector Dir = (PlayerLoc - OwnerLoc).GetSafeNormal2D();
	Owner->AddMovementInput(Dir);
}

void UFSMState_Chase::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[Chase] Exit"));
}
