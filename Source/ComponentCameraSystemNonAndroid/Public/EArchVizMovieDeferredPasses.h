// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoviePipelineDeferredPasses.h"
#include "EArchVizMovieDeferredPasses.generated.h"

/**
 * Utility classes used to support movie render queue for arch viz cameras.
 */
UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass : public UMoviePipelineDeferredPassBase
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass() : UMoviePipelineDeferredPassBase()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_FinalImage");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override 
	{ 
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_Lit", "EArchViz Deferred Rendering"); 
	}
#endif

	virtual FSceneView* GetSceneViewForSampleState(FSceneViewFamily* ViewFamily, FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload = nullptr) override;
};

UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass_Unlit : public UEArchVizMovieDeferredPass
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass_Unlit() : UEArchVizMovieDeferredPass()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_Unlit");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override
	{
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_Unlit", "EArchViz Deferred Rendering (Unlit)");
	}
#endif
	virtual void GetViewShowFlags(FEngineShowFlags& OutShowFlag, EViewModeIndex& OutViewModeIndex) const override
	{
		OutShowFlag = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
		OutViewModeIndex = EViewModeIndex::VMI_Unlit;
	}
	virtual int32 GetOutputFileSortingOrder() const override { return 2; }

};

UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass_DetailLighting : public UEArchVizMovieDeferredPass
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass_DetailLighting() : UEArchVizMovieDeferredPass()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_DetailLighting");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override
	{
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_DetailLighting", "EArchViz Deferred Rendering (Detail Lighting)");
	}
#endif
	virtual void GetViewShowFlags(FEngineShowFlags& OutShowFlag, EViewModeIndex& OutViewModeIndex) const override
	{
		OutShowFlag = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
		OutShowFlag.SetLightingOnlyOverride(true);
		OutViewModeIndex = EViewModeIndex::VMI_Lit_DetailLighting;
	}
	virtual int32 GetOutputFileSortingOrder() const override { return 2; }
};

UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass_LightingOnly : public UEArchVizMovieDeferredPass
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass_LightingOnly() : UEArchVizMovieDeferredPass()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_LightingOnly");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override
	{
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_LightingOnly", "EArchViz Deferred Rendering (Lighting Only)");
	}
#endif
	virtual void GetViewShowFlags(FEngineShowFlags& OutShowFlag, EViewModeIndex& OutViewModeIndex) const override
	{
		OutShowFlag = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
		OutShowFlag.SetLightingOnlyOverride(true);
		OutViewModeIndex = EViewModeIndex::VMI_LightingOnly;
	}
	virtual int32 GetOutputFileSortingOrder() const override { return 2; }
};

UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass_ReflectionsOnly : public UEArchVizMovieDeferredPass
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass_ReflectionsOnly() : UEArchVizMovieDeferredPass()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_ReflectionsOnly");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override
	{
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_ReflectionsOnly", "EArchViz Deferred Rendering (Reflections Only)");
	}
#endif
	virtual void GetViewShowFlags(FEngineShowFlags& OutShowFlag, EViewModeIndex& OutViewModeIndex) const override
	{
		OutShowFlag = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
		OutShowFlag.SetReflectionOverride(true);
		OutViewModeIndex = EViewModeIndex::VMI_ReflectionOverride;
	}
	virtual int32 GetOutputFileSortingOrder() const override { return 2; }
};

UCLASS(BlueprintType)
class COMPONENTCAMERASYSTEMNONANDROID_API UEArchVizMovieDeferredPass_PathTracer : public UEArchVizMovieDeferredPass
{
	GENERATED_BODY()

public:
	UEArchVizMovieDeferredPass_PathTracer() : UEArchVizMovieDeferredPass()
	{
		PassIdentifier = FMoviePipelinePassIdentifier("EArchViz_PathTracer");
	}
#if WITH_EDITOR
	virtual FText GetDisplayText() const override
	{
		return NSLOCTEXT("EArchVizMoveRenderPipeline", "EArchVizMoveRenderPipeline_DisplayName_PathTracer", "EArchViz Deferred Rendering (Path Tracer)");
	}
	virtual FText GetFooterText(UMoviePipelineExecutorJob* InJob) const override
	{
		return NSLOCTEXT(
			"EArchVizMoveRenderPipeline",
			"EArchVizMoveRenderPipeline_FooterText_PathTracer",
			"Samples per Pixel for the Path Tracer are controlled by the Spatial Sample Count from the Anti-Aliasing settings.\n"
			"All other Path Tracer settings are taken from the Post Process settings as usual.");
	}
#endif
	virtual void GetViewShowFlags(FEngineShowFlags& OutShowFlag, EViewModeIndex& OutViewModeIndex) const override
	{
		OutShowFlag = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
		OutShowFlag.SetPathTracing(true);
		OutViewModeIndex = EViewModeIndex::VMI_PathTracing;
	}
	virtual int32 GetOutputFileSortingOrder() const override { return 2; }
	virtual bool IsAntiAliasingSupported() const override { return false; }

};