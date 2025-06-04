// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComponentCameraSystemEditor : ModuleRules
{
	public ComponentCameraSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PublicIncludePaths.AddRange(
			new string[] {

			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {

			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"PropertyEditor",
				"GameplayCameras",
				"ComponentCameraSystem",
                "ActorSequence",
                "BlueprintGraph",
                "GraphEditor",
                "KismetCompiler",
                "KismetWidgets",
                "EditorStyle"
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);
	}
}
