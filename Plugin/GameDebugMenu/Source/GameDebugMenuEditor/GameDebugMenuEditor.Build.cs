/**
* Copyright (c) 2023 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

using UnrealBuildTool;

public class GameDebugMenuEditor : ModuleRules
{
	public GameDebugMenuEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		if (System.Environment.GetEnvironmentVariable("UE_COVERITY_BUILD") == "1")
		{
			PCHUsage = PCHUsageMode.NoPCHs;
			bUseUnity = false;
		}
		else
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bUseUnity = true;
		}

        PrivateIncludePaths.AddRange(
			new string[] 
            {
				"GameDebugMenuEditor/Private",
				"GameDebugMenuEditor/Private/AssetTypeActions",
                "GameDebugMenuEditor/Private/DetailCustomizations",
                "GameDebugMenuEditor/Private/Factory",
                "GameDebugMenuEditor/Private/Pins",
            }
            );
        
        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "UMG",
                "UnrealEd",
                "UMGEditor",
                "GameDebugMenu",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GraphEditor",
				"BlueprintGraph",
			}
            );
    }
}
