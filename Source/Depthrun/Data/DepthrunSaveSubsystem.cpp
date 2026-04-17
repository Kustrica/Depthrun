// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunSaveSubsystem.h"
#include "SQLiteManager.h"
#include "DepthrunLogChannels.h"
#include "Misc/Paths.h"

void UDepthrunSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DB = NewObject<USQLiteManager>(this);
	const FString DBPath = FPaths::ProjectSavedDir() / TEXT("Depthrun.db");
	if (DB->OpenDatabase(DBPath)) { InitializeSchema(); }
}

void UDepthrunSaveSubsystem::Deinitialize()
{
	if (DB) { DB->CloseDatabase(); }
	Super::Deinitialize();
}

void UDepthrunSaveSubsystem::InitializeSchema()
{
	// TODO (Stage 10): CREATE TABLE IF NOT EXISTS player_progress, run_history, adaptive_weights
	UE_LOG(LogDepthrun, Log, TEXT("[Save] InitializeSchema — stub"));
}

void UDepthrunSaveSubsystem::SaveRunResult(int32 Floor, int32 Score, bool bWon)
{
	// TODO (Stage 10): INSERT into run_history
}

bool UDepthrunSaveSubsystem::LoadProgress(int32& OutBestFloor, int32& OutTotalRuns)
{
	// TODO (Stage 10): SELECT from player_progress
	OutBestFloor  = 0;
	OutTotalRuns  = 0;
	return false;
}

void UDepthrunSaveSubsystem::SaveAdaptiveWeights(const TArray<float>& Weights, int32 RunNumber)
{
	// TODO (Stage 10): INSERT snapshot into adaptive_weights for CSV export
}
