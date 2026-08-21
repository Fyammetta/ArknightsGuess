// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ArknightsGuess : ModuleRules
{
	public ArknightsGuess(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"NetCore",
			"UMG",
			"DevNotification",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Sockets",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"DeveloperSettings",
			"SlateCore"
		});
	}
}
