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
                "Core",
                "CoreUObject",
                "Engine",
				"UMG",
                "Slate",
                "SlateCore",
                "InputCore",
                "RenderCore",
                "HeadMountedDisplay",
                "GameplayCameras",
                "CinematicCamera",
                "EnhancedInput",
                "ActorSequence",
                "MovieScene",
                "NeuralNetworkInference",
                "RHI",
                "RHICore"
				//"D3D12RHI"
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
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
