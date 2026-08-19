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
			"Networking",
			"ApplicationCore", // FAndroidApplication (UE5.4 安卓 JNI 头文件所在模块)
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"DeveloperSettings",
			"SlateCore",
		});
	}
}
