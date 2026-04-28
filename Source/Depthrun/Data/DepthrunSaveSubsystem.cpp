// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "DepthrunSaveSubsystem.h"
#include "SQLiteManager.h"
#include "DepthrunLogChannels.h"
#include "Misc/Paths.h"

void UDepthrunSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Initializing SaveSubsystem..."));
	
	DB = NewObject<USQLiteManager>(this);
	if (!DB)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[Save] Failed to create USQLiteManager!"));
		return;
	}
	
	const FString DBPath = FPaths::ProjectSavedDir() / TEXT("Depthrun.db");
	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Database path: %s"), *DBPath);
	
	if (DB->OpenDatabase(DBPath))
	{
		UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Database opened successfully"));
		InitializeSchema();
	}
	else
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[Save] Failed to open database at %s"), *DBPath);
	}
}

void UDepthrunSaveSubsystem::Deinitialize()
{
	if (DB) { DB->CloseDatabase(); }
	Super::Deinitialize();
}

void UDepthrunSaveSubsystem::InitializeSchema()
{
	if (!DB) { return; }

	// player_profile table: 1 row with TotalDiamonds and upgrade levels
	const FString CreateProfileTable = TEXT(
		"CREATE TABLE IF NOT EXISTS player_profile ("
		"id INTEGER PRIMARY KEY CHECK (id = 1),"
		"TotalDiamonds INTEGER DEFAULT 0,"
		"Damage_Lvl INTEGER DEFAULT 0,"
		"Range_Lvl INTEGER DEFAULT 0,"
		"ArrowCount_Lvl INTEGER DEFAULT 0,"
		"MaxHP_Lvl INTEGER DEFAULT 0"
		");"
	);
	DB->ExecuteQuery(CreateProfileTable);

	// Insert default row if not exists
	const FString InsertDefault = TEXT(
		"INSERT OR IGNORE INTO player_profile (id, TotalDiamonds, Damage_Lvl, Range_Lvl, ArrowCount_Lvl, MaxHP_Lvl) "
		"VALUES (1, 0, 0, 0, 0, 0);"
	);
	DB->ExecuteQuery(InsertDefault);

	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Schema initialized — player_profile ready"));
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

// ─── Player Profile Implementation ────────────────────────────────────────────

int32 UDepthrunSaveSubsystem::GetTotalDiamonds() const
{
	if (!DB) { return 0; }

	TArray<TMap<FString, FString>> Rows = DB->SelectRows(TEXT("player_profile"), TEXT("id = 1"));
	if (Rows.Num() > 0 && Rows[0].Contains(TEXT("TotalDiamonds")))
	{
		return FCString::Atoi(*Rows[0][TEXT("TotalDiamonds")]);
	}
	return 0;
}

void UDepthrunSaveSubsystem::AddDiamondsToProfile(int32 Amount)
{
	if (!DB || Amount <= 0) { return; }

	int32 Current = GetTotalDiamonds();
	int32 NewTotal = Current + Amount;

	FString Query = FString::Printf(
		TEXT("UPDATE player_profile SET TotalDiamonds = %d WHERE id = 1;"),
		NewTotal
	);
	DB->ExecuteQuery(Query);

	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Diamonds: %d → %d (+%d)"), Current, NewTotal, Amount);
}

int32 UDepthrunSaveSubsystem::GetUpgradeLevel(EHubUpgrade Type) const
{
	if (!DB) { return 0; }

	const TCHAR* ColumnName = TEXT("");
	switch (Type)
	{
		case EHubUpgrade::Damage:     ColumnName = TEXT("Damage_Lvl"); break;
		case EHubUpgrade::Range:      ColumnName = TEXT("Range_Lvl"); break;
		case EHubUpgrade::ArrowCount: ColumnName = TEXT("ArrowCount_Lvl"); break;
		case EHubUpgrade::MaxHP:      ColumnName = TEXT("MaxHP_Lvl"); break;
	}

	TArray<TMap<FString, FString>> Rows = DB->SelectRows(TEXT("player_profile"), TEXT("id = 1"));
	if (Rows.Num() > 0 && Rows[0].Contains(ColumnName))
	{
		return FCString::Atoi(*Rows[0][ColumnName]);
	}
	return 0;
}

int32 UDepthrunSaveSubsystem::GetUpgradeCost(EHubUpgrade Type) const
{
	int32 CurrentLevel = GetUpgradeLevel(Type);
	return HubUpgradeConfig::GetUpgradeCost(CurrentLevel);
}

bool UDepthrunSaveSubsystem::BuyUpgrade(EHubUpgrade Type)
{
	if (!DB) { return false; }

	int32 CurrentLevel = GetUpgradeLevel(Type);
	if (CurrentLevel >= HubUpgradeConfig::MAX_LEVEL)
	{
		UE_LOG(LogDepthrunSave, Warning, TEXT("[Save] BuyUpgrade failed: %s already at max level %d"),
			*UEnum::GetValueAsString(Type), CurrentLevel);
		return false;
	}

	int32 Cost = HubUpgradeConfig::GetUpgradeCost(CurrentLevel);
	int32 CurrentDiamonds = GetTotalDiamonds();

	if (CurrentDiamonds < Cost)
	{
		UE_LOG(LogDepthrunSave, Log, TEXT("[Save] BuyUpgrade failed: need %d diamonds, have %d"),
			Cost, CurrentDiamonds);
		return false;
	}

	// Deduct diamonds and upgrade
	int32 NewDiamonds = CurrentDiamonds - Cost;
	int32 NewLevel = CurrentLevel + 1;

	const TCHAR* ColumnName = TEXT("");
	switch (Type)
	{
		case EHubUpgrade::Damage:     ColumnName = TEXT("Damage_Lvl"); break;
		case EHubUpgrade::Range:      ColumnName = TEXT("Range_Lvl"); break;
		case EHubUpgrade::ArrowCount: ColumnName = TEXT("ArrowCount_Lvl"); break;
		case EHubUpgrade::MaxHP:      ColumnName = TEXT("MaxHP_Lvl"); break;
	}

	FString Query = FString::Printf(
		TEXT("UPDATE player_profile SET TotalDiamonds = %d, %s = %d WHERE id = 1;"),
		NewDiamonds, ColumnName, NewLevel
	);
	DB->ExecuteQuery(Query);

	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Upgrade %s: level %d → %d, diamonds %d → %d"),
		*UEnum::GetValueAsString(Type), CurrentLevel, NewLevel, CurrentDiamonds, NewDiamonds);
	return true;
}

void UDepthrunSaveSubsystem::ResetProfile()
{
	if (!DB) { return; }

	const FString Query = TEXT(
		"UPDATE player_profile SET TotalDiamonds = 0, Damage_Lvl = 0, Range_Lvl = 0, "
		"ArrowCount_Lvl = 0, MaxHP_Lvl = 0 WHERE id = 1;"
	);
	DB->ExecuteQuery(Query);

	UE_LOG(LogDepthrunSave, Log, TEXT("[Save] Profile reset to defaults"));
}
