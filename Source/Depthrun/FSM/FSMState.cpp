// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState.h"

void UFSMState::EnterState(ABaseEnemy* Owner)
{
	TimeInState = 0.f;
}

void UFSMState::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	TimeInState += DeltaTime;
}

void UFSMState::ExitState(ABaseEnemy* Owner)
{
	// Base: nothing to clean up. Overridden by concrete states if needed.
}
