// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "SQLiteManager.h"
#include "DepthrunLogChannels.h"

bool USQLiteManager::OpenDatabase(const FString& Path)
{
	// TODO (Stage 10): FSQLiteDatabase::Open(Path)
	UE_LOG(LogDepthrun, Log, TEXT("[SQLite] OpenDatabase(%s) — stub"), *Path);
	bIsOpen = true; // placeholder
	return true;
}

void USQLiteManager::CloseDatabase()
{
	bIsOpen = false;
}

bool USQLiteManager::ExecuteQuery(const FString& SQL)
{
	// TODO (Stage 10): DbHandle.Execute(SQL)
	return true;
}

int64 USQLiteManager::InsertRow(const FString& Table, const TMap<FString, FString>& Columns)
{
	// TODO (Stage 10): build INSERT SQL, execute, return last row id
	return -1;
}

TArray<TMap<FString, FString>> USQLiteManager::SelectRows(const FString& Table, const FString& WhereClause)
{
	// TODO (Stage 10): build SELECT SQL, execute, convert result set
	return {};
}
