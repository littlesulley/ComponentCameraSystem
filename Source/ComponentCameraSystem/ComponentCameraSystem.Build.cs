// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComponentCameraSystem : ModuleRules
{
	public ComponentCameraSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"Core"
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "InputCore",
				"HeadMountedDisplay",
                "GameplayCameras",
				"CinematicCamera",
				"EnhancedInput",
				"ActorSequence",
				"MovieScene"            
			}
            );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
