// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Chase.h"
#include "DepthrunLogChannels.h"

void UFSMState_Chase::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Chase → Enter"));
	// TODO (Stage 5B): play run animation
}

void UFSMState_Chase::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	// TODO (Stage 5B): MoveToward player, check AttackRangeThreshold
}

void UFSMState_Chase::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Chase → Exit"));
}
