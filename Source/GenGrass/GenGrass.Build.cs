// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class GenGrass : ModuleRules
{
	public GenGrass(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { });
        PrivateDependencyModuleNames.AddRange(new string[]
        {
                "Core",
                "CoreUObject",
                "Engine",
                "Renderer",
                "RenderCore",
                "RHI",
                "Projects"
        });
  //      PublicIncludePaths.AddRange(new string[] {"GenGrass/Public"});
 
		//PrivateIncludePaths.AddRange(new string[] {"GenGrass/Private"});


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
