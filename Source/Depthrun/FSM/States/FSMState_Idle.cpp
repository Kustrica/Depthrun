// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Idle.h"
#include "DepthrunLogChannels.h"

void UFSMState_Idle::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	TimeSinceLastLook = 0.f;
	UE_LOG(LogFSM, Verbose, TEXT("[State] Idle → Enter"));
	// TODO (Stage 5B): play idle animation
}

void UFSMState_Idle::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	TimeSinceLastLook += DeltaTime;
	// TODO (Stage 5B): look-around trigger, re-evaluation request
}

void UFSMState_Idle::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Idle → Exit"));
}
