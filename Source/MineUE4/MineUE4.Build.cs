// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MineUE4 : ModuleRules
{
	public MineUE4(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent" });
		
        string ThirdParty = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty"));

        string LibraryEnetPath = Path.Combine(ThirdParty, "enet", "lib"); // my lib dir
        string IncludeEnetPath = Path.Combine(ThirdParty, "enet", "include"); // my include dir
		
		string LibraryFastNoisePath = Path.Combine(ThirdParty, "FastNoise", "lib"); // my lib dir
        string IncludeFastNoisePath = Path.Combine(ThirdParty, "FastNoise", "include"); // my include dir

        //include the stuff
        PublicIncludePaths.Add(IncludeEnetPath); // enet
        PublicAdditionalLibraries.Add(LibraryEnetPath + "/enet.lib"); // enet
		
		PublicIncludePaths.Add(IncludeFastNoisePath); // FastNoise
        PublicAdditionalLibraries.Add(LibraryFastNoisePath + "/FastNoise.lib"); // FastNoise

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
