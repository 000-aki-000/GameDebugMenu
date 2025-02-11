/**
* Copyright (c) 2020 akihiko moroi
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
                "HTTP",
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
				"DeveloperSettings",
			}
			);
	}
}
