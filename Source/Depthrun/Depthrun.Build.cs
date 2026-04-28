// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Depthrun : ModuleRules
{
	public Depthrun(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// UE 5.7 IWYU mode does NOT automatically add the module source root
		// to include paths. We add it explicitly so that cross-directory
		// includes like "Core/DepthrunLogChannels.h" resolve correctly.
		PrivateIncludePaths.AddRange(new string[] {
			ModuleDirectory,
			System.IO.Path.Combine(ModuleDirectory, "Audio"),
			System.IO.Path.Combine(ModuleDirectory, "Core"),
			System.IO.Path.Combine(ModuleDirectory, "Data"),
			System.IO.Path.Combine(ModuleDirectory, "Player"),
			System.IO.Path.Combine(ModuleDirectory, "RoomGeneration")
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Paper2D",
			"Niagara",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			// SQLiteCore: low-level sqlite3 C API + Epic's FSQLiteDatabase wrapper
			//             (PublicIncludePaths -> ThirdParty/, so use "sqlite/sqlite3.h")
			// SQLiteSupport: optional Blueprint-exposed nodes (USQLiteDatabase asset).
			// NOTE: there is NO module called "SQLite" in UE 5.7 — adding it breaks the build.
			"SQLiteCore",
			"SQLiteSupport"
		});
	}
}
