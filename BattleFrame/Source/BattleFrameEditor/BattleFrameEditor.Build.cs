/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

using UnrealBuildTool;

public class BattleFrameEditor : ModuleRules
{
    public BattleFrameEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "Engine", "CoreUObject", "BattleFrame", "Niagara", "NiagaraCore" });
           
        PrivateDependencyModuleNames.AddRange(new string[] {             
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "AssetTools",
            "BlueprintGraph" 
        });
    }
}
