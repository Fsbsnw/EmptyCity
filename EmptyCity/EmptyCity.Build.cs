// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EmptyCity : ModuleRules
{
	public EmptyCity(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"MotionWarping",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
			"AIModule",
			"NavigationSystem",
			"Niagara",
			"ModularGameplay",
			"ModelViewViewModel",
			"Slate", 
			"SlateCore",
			"MediaAssets",
			"LevelSequence"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
