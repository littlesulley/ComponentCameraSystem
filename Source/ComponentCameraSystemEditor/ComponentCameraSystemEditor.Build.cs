// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComponentCameraSystemEditor : ModuleRules
{
	public ComponentCameraSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PublicIncludePaths.AddRange(
			new string[] {
				//"ComponentCameraSystemEditor/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				//"ComponentCameraSystemEditor/Private"
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

        PublicDefinitions.Add("ECAMERA_K2NODE_GETPROPERTYBYNAME");
    }
}
