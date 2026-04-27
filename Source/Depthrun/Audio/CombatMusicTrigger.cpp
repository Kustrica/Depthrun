// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Audio/CombatMusicTrigger.h"
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "RoomGeneration/RoomGeneratorSubsystem.h"
#include "RoomGeneration/RoomBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"
#include "DepthrunLogChannels.h"

UCombatMusicTrigger::UCombatMusicTrigger()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatMusicTrigger::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(
		EvalTimerHandle,
		this,
		&UCombatMusicTrigger::EvaluateCombatState,
		PollInterval,
		true);
}

void UCombatMusicTrigger::EndPlay(const EEndPlayReason::Type Reason)
{
	GetWorld()->GetTimerManager().ClearTimer(EvalTimerHandle);
	Super::EndPlay(Reason);
}

void UCombatMusicTrigger::EvaluateCombatState()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	URoomGeneratorSubsystem* RoomGen = World->GetSubsystem<URoomGeneratorSubsystem>();
	if (!RoomGen) { return; }

	ARoomBase* ActiveRoom = RoomGen->GetCurrentActiveRoom();
	bool bInCombat = IsValid(ActiveRoom) && ActiveRoom->IsCombatActive();

	UMusicSubsystem* Music = UGameplayStatics::GetGameInstance(World)
		? UGameplayStatics::GetGameInstance(World)->GetSubsystem<UMusicSubsystem>()
		: nullptr;

	if (!Music) { return; }

	if (bInCombat)
	{
		CombatEndTime = -999.f;
		if (!bWasInCombat)
		{
			bWasInCombat = true;
			UE_LOG(LogDepthrunMusic, Log, TEXT("[CombatTrigger] Combat started — switching to Combat track"));
			Music->PlayMusic(EMusicTrack::Combat, 0.8f, 0.8f);
		}
	}
	else
	{
		if (bWasInCombat)
		{
			// Start debounce timer
			if (CombatEndTime < 0.f)
			{
				CombatEndTime = World->GetTimeSeconds();
				UE_LOG(LogDepthrunMusic, Log, TEXT("[CombatTrigger] Combat ended — debounce started (%.1fs)"), CombatEndDebounce);
			}

			float Elapsed = World->GetTimeSeconds() - CombatEndTime;
			if (Elapsed >= CombatEndDebounce)
			{
				bWasInCombat = false;
				CombatEndTime = -999.f;
				UE_LOG(LogDepthrunMusic, Log, TEXT("[CombatTrigger] Debounce complete — switching to Explore track"));
				Music->PlayMusic(EMusicTrack::Explore, 2.0f, 2.0f);
			}
		}
	}
}
