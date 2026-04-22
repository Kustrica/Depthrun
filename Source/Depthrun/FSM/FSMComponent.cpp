// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "FSMComponent.h"
#include "FSMState.h"
#include "Enemy/BaseEnemy.h"
#include "DepthrunLogChannels.h"

UFSMComponent::UFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerEnemy = Cast<ABaseEnemy>(GetOwner());
}

void UFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState && OwnerEnemy)
	{
		CurrentState->TickState(OwnerEnemy, DeltaTime);
	}
}

void UFSMComponent::RegisterState(EFSMStateType StateType, UFSMState* StateObject)
{
	if (!StateObject)
	{
		UE_LOG(LogFSM, Warning, TEXT("UFSMComponent::RegisterState — null state for type %d"), (int32)StateType);
		return;
	}
	States.Add(StateType, StateObject);
	UE_LOG(LogFSM, Verbose, TEXT("UFSMComponent: registered state %d"), (int32)StateType);
}

void UFSMComponent::TransitionTo(EFSMStateType NewState)
{
	// TODO (Stage 5): implement full transition logic
	// Exit current → enter new → broadcast delegate

	if (NewState == CurrentStateType) return;

	const EFSMStateType OldState = CurrentStateType;

	if (CurrentState && OwnerEnemy)
	{
		CurrentState->ExitState(OwnerEnemy);
	}

	auto Found = States.Find(NewState);
	if (!Found || !(*Found))
	{
		UE_LOG(LogFSM, Warning, TEXT("UFSMComponent::TransitionTo — state %d not registered!"), (int32)NewState);
		return;
	}

	CurrentState     = *Found;
	CurrentStateType = NewState;

	if (CurrentState && OwnerEnemy)
	{
		CurrentState->EnterState(OwnerEnemy);
	}

	const FString OldName = GetStateName(OldState);
	const FString NewName = GetStateName(NewState);
	UE_LOG(LogFSM, Log, TEXT("FSM[%s]: %s → %s"), *GetOwner()->GetName(), *OldName, *NewName);
	OnStateChanged.Broadcast(OldState, NewState);
}

float UFSMComponent::GetTimeInCurrentState() const
{
	return CurrentState ? CurrentState->GetTimeInState() : 0.f;
}

FString UFSMComponent::GetStateName(EFSMStateType State)
{
	const UEnum* EnumPtr = StaticEnum<EFSMStateType>();
	if (!EnumPtr) return TEXT("Unknown");
	return EnumPtr->GetDisplayNameTextByValue((int64)State).ToString();
}
