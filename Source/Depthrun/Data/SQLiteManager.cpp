// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "SQLiteManager.h"
#include "Core/DepthrunLogChannels.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// SQLiteCore exposes ThirdParty/ as a public include path.
// The sqlite3 amalgamation header sits in ThirdParty/sqlite/sqlite3.h,
// so the correct include is "sqlite/sqlite3.h" (matches Epic's own IncludeSQLite.h).
#include "sqlite/sqlite3.h"

bool USQLiteManager::OpenDatabase(const FString& Path)
{
	if (bIsOpen && SQLiteDB)
	{
		UE_LOG(LogDepthrun, Warning, TEXT("[SQLite] Database already open, closing first"));
		CloseDatabase();
	}

	// Ensure directory exists
	FString DirPath = FPaths::GetPath(Path);
	if (!FPaths::DirectoryExists(DirPath))
	{
		IFileManager::Get().MakeDirectory(*DirPath, true);
	}

	const char* PathUTF8 = TCHAR_TO_UTF8(*Path);
	sqlite3* DB = nullptr;
	int Result = sqlite3_open(PathUTF8, &DB);

	if (Result != SQLITE_OK)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] Failed to open database: %s"), UTF8_TO_TCHAR(sqlite3_errstr(Result)));
		if (DB) sqlite3_close(DB);
		return false;
	}

	SQLiteDB = DB;
	bIsOpen = true;
	UE_LOG(LogDepthrunSave, Log, TEXT("[SQLite] Database opened: %s"), *Path);
	return true;
}

void USQLiteManager::CloseDatabase()
{
	if (SQLiteDB)
	{
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
	}
	bIsOpen = false;
	UE_LOG(LogDepthrunSave, Log, TEXT("[SQLite] Database closed"));
}

bool USQLiteManager::ExecuteQuery(const FString& SQL)
{
	if (!bIsOpen || !SQLiteDB)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] ExecuteQuery failed: database not open"));
		return false;
	}

	const char* SQLUTF8 = TCHAR_TO_UTF8(*SQL);
	char* ErrorMsg = nullptr;

	int Result = sqlite3_exec(SQLiteDB, SQLUTF8, nullptr, nullptr, &ErrorMsg);

	if (Result != SQLITE_OK)
	{
		if (ErrorMsg)
		{
			UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] SQL error: %s"), UTF8_TO_TCHAR(ErrorMsg));
			sqlite3_free(ErrorMsg);
		}
		return false;
	}

	UE_LOG(LogDepthrunSave, Verbose, TEXT("[SQLite] Executed: %s"), *SQL);
	return true;
}

int64 USQLiteManager::InsertRow(const FString& Table, const TMap<FString, FString>& Columns)
{
	if (!bIsOpen || !SQLiteDB || Columns.Num() == 0)
	{
		return -1;
	}

	FString ColumnNames, ValuePlaceholders;
	TArray<FString> Values;

	bool bFirst = true;
	for (const auto& Pair : Columns)
	{
		if (!bFirst)
		{
			ColumnNames += TEXT(", ");
			ValuePlaceholders += TEXT(", ");
		}
		ColumnNames += Pair.Key;
		ValuePlaceholders += TEXT("?");
		Values.Add(Pair.Value);
		bFirst = false;
	}

	FString SQL = FString::Printf(TEXT("INSERT INTO %s (%s) VALUES (%s);"), *Table, *ColumnNames, *ValuePlaceholders);
	const char* SQLUTF8 = TCHAR_TO_UTF8(*SQL);

	sqlite3_stmt* Stmt = nullptr;
	if (sqlite3_prepare_v2(SQLiteDB, SQLUTF8, -1, &Stmt, nullptr) != SQLITE_OK)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] Prepare failed: %s"), UTF8_TO_TCHAR(sqlite3_errmsg(SQLiteDB)));
		return -1;
	}

	for (int32 i = 0; i < Values.Num(); ++i)
	{
		const char* ValueUTF8 = TCHAR_TO_UTF8(*Values[i]);
		sqlite3_bind_text(Stmt, i + 1, ValueUTF8, -1, SQLITE_TRANSIENT);
	}

	int Result = sqlite3_step(Stmt);
	sqlite3_finalize(Stmt);

	if (Result != SQLITE_DONE)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] Insert failed: %s"), UTF8_TO_TCHAR(sqlite3_errmsg(SQLiteDB)));
		return -1;
	}

	return sqlite3_last_insert_rowid(SQLiteDB);
}

TArray<TMap<FString, FString>> USQLiteManager::SelectRows(const FString& Table, const FString& WhereClause)
{
	TArray<TMap<FString, FString>> Result;

	if (!bIsOpen || !SQLiteDB)
	{
		return Result;
	}

	FString SQL = FString::Printf(TEXT("SELECT * FROM %s"), *Table);
	if (!WhereClause.IsEmpty())
	{
		SQL += FString::Printf(TEXT(" WHERE %s"), *WhereClause);
	}
	SQL += TEXT(";");

	const char* SQLUTF8 = TCHAR_TO_UTF8(*SQL);
	sqlite3_stmt* Stmt = nullptr;

	if (sqlite3_prepare_v2(SQLiteDB, SQLUTF8, -1, &Stmt, nullptr) != SQLITE_OK)
	{
		UE_LOG(LogDepthrunSave, Error, TEXT("[SQLite] Select prepare failed: %s"), UTF8_TO_TCHAR(sqlite3_errmsg(SQLiteDB)));
		return Result;
	}

	int ColCount = sqlite3_column_count(Stmt);

	while (sqlite3_step(Stmt) == SQLITE_ROW)
	{
		TMap<FString, FString> Row;
		for (int i = 0; i < ColCount; ++i)
		{
			FString ColName = UTF8_TO_TCHAR(sqlite3_column_name(Stmt, i));
			const char* Value = (const char*)sqlite3_column_text(Stmt, i);
			FString ValueStr = Value ? UTF8_TO_TCHAR(Value) : TEXT("");
			Row.Add(ColName, ValueStr);
		}
		Result.Add(Row);
	}

	sqlite3_finalize(Stmt);
	return Result;
}
