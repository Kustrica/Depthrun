// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMState_Flank.h"
#include "DepthrunLogChannels.h"

void UFSMState_Flank::EnterState(ABaseEnemy* Owner)
{
	Super::EnterState(Owner);
	TimeSinceDirectionRefresh = 0.f;
	FlankSide = (FMath::RandBool() ? 1 : -1);
	FlankDirection = FVector::ZeroVector;
	UE_LOG(LogFSM, Verbose, TEXT("[State] Flank → Enter (side=%d)"), FlankSide);
	// TODO (Stage 5B): calculate initial perpendicular direction
}

void UFSMState_Flank::TickState(ABaseEnemy* Owner, float DeltaTime)
{
	Super::TickState(Owner, DeltaTime);
	TimeSinceDirectionRefresh += DeltaTime;
	// TODO (Stage 5B): strafe perpendicular to player, refresh direction periodically
}

void UFSMState_Flank::ExitState(ABaseEnemy* Owner)
{
	Super::ExitState(Owner);
	UE_LOG(LogFSM, Verbose, TEXT("[State] Flank → Exit"));
}
