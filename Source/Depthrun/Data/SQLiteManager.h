// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SQLiteManager.generated.h"

// Forward declaration - sqlite3 from SQLiteCore module
struct sqlite3;

/**
 * USQLiteManager
 * Thin wrapper around sqlite3 (UE5 built-in SQLiteCore plugin).
 * Used by UDepthrunSaveSubsystem to persist player profile data.
 */
UCLASS()
class DEPTHRUN_API USQLiteManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Open or create a SQLite DB at the given path.
	 * @param Path  Absolute path, e.g. FPaths::ProjectSavedDir() / "Depthrun.db"
	 */
	bool OpenDatabase(const FString& Path);

	/** Close the database connection. */
	void CloseDatabase();

	/** Execute a raw SQL statement (DDL or DML). Returns true on success. */
	bool ExecuteQuery(const FString& SQL);

	/** Insert a row into a table. Returns row ID or -1 on failure. */
	int64 InsertRow(const FString& Table, const TMap<FString, FString>& Columns);

	/**
	 * Select rows matching a WHERE clause.
	 * Returns an array of column→value maps.
	 */
	TArray<TMap<FString, FString>> SelectRows(const FString& Table, const FString& WhereClause = TEXT(""));

	bool IsOpen() const { return bIsOpen; }

private:
	bool bIsOpen = false;
	sqlite3* SQLiteDB = nullptr;
};
