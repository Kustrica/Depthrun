// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Retreat.h"
#include "DepthrunLogChannels.h"

void UFSMState_Retreat::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	RetreatDirection = FVector::ZeroVector;
	UE_LOG(LogFSM, Verbose, TEXT("[State] Retreat → Enter"));
	// TODO (Stage 5B): calculate initial safe retreat direction
}

void UFSMState_Retreat::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	// TODO (Stage 5B): move away from player, obstacle-avoidance check, refresh direction
}

void UFSMState_Retreat::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Retreat → Exit"));
}
