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
			System.IO.Path.Combine(ModuleDirectory, "Core")
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
			"SQLiteCore",        // built-in UE5 SQLite wrapper
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
	}
}
