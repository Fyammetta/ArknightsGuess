// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ArknightsGuess : ModuleRules
{
	public ArknightsGuess(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NetCore",
			"UMG",
			"DevNotification",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"DeveloperSettings",
			"SlateCore",
		});
	}
}
