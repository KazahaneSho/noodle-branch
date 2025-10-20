// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class NoodleBranch : ModuleRules
{
	public NoodleBranch(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange([
			"Core", 
			"CoreOnline",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"GameFeatures", 
			"AIModule",
			"ModularGameplay", 
			"ModularGameplayActors",
			"CommonLoadingScreen"
		]);

		PrivateDependencyModuleNames.AddRange([
			"CommonGame",
			"CommonUser",
			"DeveloperSettings",
			"GameSettings",
			"EngineSettings"
		]);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
