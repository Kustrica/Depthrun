// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Attack.h"
#include "Enemy/AdaptiveEnemy.h"
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

	// Ranged enemies keep moving — only stop melee enemies during attack.
	AAdaptiveEnemy* Adaptive = Cast<AAdaptiveEnemy>(Owner);
	bool bIsRanged = Adaptive && Adaptive->bIsRangedMode;
	if (!bIsRanged && Owner && Owner->GetCharacterMovement())
	{
		Owner->GetCharacterMovement()->StopMovementImmediately();
	}

	UE_LOG(LogFSM, Log, TEXT("[Attack] Enter — %s (ranged=%s)"), *GetNameSafe(Owner), bIsRanged ? TEXT("true") : TEXT("false"));
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

	// Check if this is a ranged adaptive enemy
	AAdaptiveEnemy* Adaptive = Cast<AAdaptiveEnemy>(Owner);
	const bool bIsRanged = Adaptive && Adaptive->bIsRangedMode;

	// ── Ranged: back off if player is too close ────────────────────────────
	if (bIsRanged && Dist < Owner->MinAttackRange)
	{
		FSM->TransitionTo(EFSMStateType::Retreat);
		return;
	}

	// ── Exit condition: player moved out of attack range ───────────────────
	// Ranged: exit to Chase if player goes past RangedTooFarRange — this lets
	// the enemy actually close the distance instead of strafing forever.
	// Without this, ranged Attack "stuck" at long distance because the previous
	// ExitRange = DetectionRange*0.9 was almost never reached.
	// Melee: exit to Chase when player walks away past MeleeAttackRange.
	const float ExitRange = bIsRanged
		? Owner->RangedTooFarRange
		: Owner->MeleeAttackRange * 1.2f;
	if (TimeInAttackState >= MinTimeBeforeDistanceExit && Dist > ExitRange)
	{
		FSM->TransitionTo(EFSMStateType::Chase);
		return;
	}

	// ── Attack cooldown ────────────────────────────────────────────────────
	if (TimeSinceLastAttack >= AttackCooldown)
	{
		TimeSinceLastAttack = 0.f;
		Owner->PerformMeleeAttack(); // Delegate actual attack to the enemy actor
		UE_LOG(LogFSM, Log, TEXT("[Attack] %s → attack fired (ranged=%s)"), *GetNameSafe(Owner), bIsRanged ? TEXT("true") : TEXT("false"));
	}

	// ── Ranged: strafe sideways to avoid standing still ───────────────────
	if (bIsRanged)
	{
		const FVector ToPlayer   = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
		const FVector Strafe     = FVector(-ToPlayer.Y, ToPlayer.X, 0.f); // perpendicular
		Owner->AddMovementInput(Strafe, 0.4f);
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
