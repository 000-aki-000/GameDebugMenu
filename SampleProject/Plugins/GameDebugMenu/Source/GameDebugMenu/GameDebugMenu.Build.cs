/**
* Copyright (c) 2022 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

using UnrealBuildTool;
using System.IO;

public class GameDebugMenu : ModuleRules
{
	public GameDebugMenu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
			new string[] 
            {
				"GameDebugMenu/Private",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "UMG",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "Http",
                "Json",
                "JsonUtilities",
                "ImageDownload",
				"ImageWrapper",
				"EngineSettings"
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
