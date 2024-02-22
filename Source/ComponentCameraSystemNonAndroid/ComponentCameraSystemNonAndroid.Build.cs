// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComponentCameraSystemNonAndroid : ModuleRules
{
	public ComponentCameraSystemNonAndroid(ReadOnlyTargetRules Target) : base(Target)
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

            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RenderCore",
                "CinematicCamera",
                "MovieRenderPipelineCore",
                "MovieRenderPipelineRenderPasses"
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
