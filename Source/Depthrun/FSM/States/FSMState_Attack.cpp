// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Attack.h"
#include "DepthrunLogChannels.h"

void UFSMState_Attack::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	TimeSinceLastAttack = AttackCooldown; // allow immediate first attack
	bAttackCoolingDown  = false;
	UE_LOG(LogFSM, Verbose, TEXT("[State] Attack → Enter"));
	// TODO (Stage 5B): play attack animation
}

void UFSMState_Attack::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	TimeSinceLastAttack += DeltaTime;
	// TODO (Stage 5B): perform attack when cooldown elapsed; respect enemy type (Melee/Ranged)
}

void UFSMState_Attack::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Attack → Exit"));
}
