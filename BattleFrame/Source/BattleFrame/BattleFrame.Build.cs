/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

using UnrealBuildTool;

public class BattleFrame : ModuleRules
{
	public BattleFrame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Niagara",
                "ApparatusRuntime",
                "FlowFieldCanvas",
                "AnimToTexture"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
	}
}
