/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

using UnrealBuildTool;

public class GameDebugMenuEditor : ModuleRules
{
	public GameDebugMenuEditor(ReadOnlyTargetRules Target) : base(Target)
	{
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
