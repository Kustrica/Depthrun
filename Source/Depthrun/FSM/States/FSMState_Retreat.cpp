// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Retreat.h"
#include "Enemy/BaseEnemy.h"
#include "FSM/FSMComponent.h"
#include "Core/DepthrunLogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DepthrunCharacter.h"

// How often (seconds) to recalculate retreat direction (avoid walls better).
static constexpr float kRetreatRecalcInterval = 0.6f;

void UFSMState_Retreat::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);

	// Calculate initial retreat direction (directly away from player).
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (IsValid(Player) && Owner)
	{
		RetreatDirection = (Owner->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
	}

	UE_LOG(LogFSM, Log, TEXT("[Retreat] Enter — %s  dir=(%.1f,%.1f)"),
		*GetNameSafe(Owner), RetreatDirection.X, RetreatDirection.Y);
}

void UFSMState_Retreat::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	if (!Owner) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Owner, 0);
	if (!IsValid(Player)) return;

	// Commercial Fix: stop retreating if player is dead
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

	// ── Stop retreating when safe distance achieved ────────────────────────
	if (Dist >= SafeDistance)
	{
		FSM->TransitionTo(EFSMStateType::Idle);
		return;
	}

	// ── Periodically refresh retreat direction (steer around walls) ────────
	if (FMath::Fmod(TimeInState, kRetreatRecalcInterval) < DeltaTime)
	{
		RetreatDirection = (Owner->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
	}

	Owner->AddMovementInput(RetreatDirection);
}

void UFSMState_Retreat::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[Retreat] Exit"));
}
