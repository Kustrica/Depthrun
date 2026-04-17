// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState.h"
#include "DepthrunLogChannels.h"

void UFSMState::EnterState(ABaseEnemy* Owner)
{
	TimeInState = 0.f;
	// TODO (Stage 5): implement per-state enter logic
}

void UFSMState::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	TimeInState += DeltaTime;
	// TODO (Stage 5): implement per-state tick logic
}

void UFSMState::ExitState(ABaseEnemy* Owner)
{
	// TODO (Stage 5): implement per-state exit logic
}
