// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DevNotification : ModuleRules
{
	public DevNotification(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings",
			"UMG",
		});
	}
}
