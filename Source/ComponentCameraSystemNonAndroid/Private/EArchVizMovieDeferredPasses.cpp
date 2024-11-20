// Copyright 2023 by Sulley. All Rights Reserved.

#include "EArchVizMovieDeferredPasses.h"
#include "MoviePipeline.h"

FSceneView* UEArchVizMovieDeferredPass::GetSceneViewForSampleState(FSceneViewFamily* ViewFamily, FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload)
{
	APlayerController* LocalPlayerController = GetPipeline()->GetWorld()->GetFirstPlayerController();

	APlayerCameraManager* PlayerCameraManager = GetPipeline()->GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	FMinimalViewInfo POV = PlayerCameraManager->GetCameraCacheView();

	double OffsetX = POV.OffCenterProjectionOffset.X;
	double OffsetY = POV.OffCenterProjectionOffset.Y;

	int32 TileSizeX = InOutSampleState.BackbufferSize.X;
	int32 TileSizeY = InOutSampleState.BackbufferSize.Y;

	UE::MoviePipeline::FImagePassCameraViewData CameraInfo = GetCameraInfo(InOutSampleState, OptPayload);

	const float DestAspectRatio = InOutSampleState.BackbufferSize.X / (float)InOutSampleState.BackbufferSize.Y;
	const float CameraAspectRatio = bAllowCameraAspectRatio ? CameraInfo.ViewInfo.AspectRatio : DestAspectRatio;

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.ViewOrigin = CameraInfo.ViewInfo.Location;
	ViewInitOptions.SetViewRectangle(FIntRect(FIntPoint(0, 0), FIntPoint(TileSizeX, TileSizeY)));
	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(CameraInfo.ViewInfo.Rotation);
	ViewInitOptions.ViewActor = CameraInfo.ViewActor;

	// Rotate the view 90 degrees (reason: unknown)
	ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	float ViewFOV = CameraInfo.ViewInfo.FOV;

	// Inflate our FOV to support the overscan 
	ViewFOV = 2.0f * FMath::RadiansToDegrees(FMath::Atan((1.0f + InOutSampleState.OverscanPercentage) * FMath::Tan(FMath::DegreesToRadians(ViewFOV * 0.5f))));

	float DofSensorScale = 1.0f;

	// Calculate a Projection Matrix. This code unfortunately ends up similar to, but not quite the same as FMinimalViewInfo::CalculateProjectionMatrixGivenView
	FMatrix BaseProjMatrix;

	if (CameraInfo.bUseCustomProjectionMatrix)
	{
		BaseProjMatrix = CameraInfo.CustomProjectionMatrix;

		// Modify the custom matrix to do an off center projection, with overlap for high-res tiling
		const bool bOrthographic = false;
		ModifyProjectionMatrixForTiling(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale);
	}
	else
	{
		if (CameraInfo.ViewInfo.ProjectionMode == ECameraProjectionMode::Orthographic)
		{
			const float YScale = 1.0f / CameraAspectRatio;
			const float OverscanScale = 1.0f + InOutSampleState.OverscanPercentage;

			const float HalfOrthoWidth = (CameraInfo.ViewInfo.OrthoWidth / 2.0f) * OverscanScale;
			const float ScaledOrthoHeight = (CameraInfo.ViewInfo.OrthoWidth / 2.0f) * OverscanScale * YScale;

			const float NearPlane = CameraInfo.ViewInfo.OrthoNearClipPlane;
			const float FarPlane = CameraInfo.ViewInfo.OrthoFarClipPlane;

			const float ZScale = 1.0f / (FarPlane - NearPlane);
			const float ZOffset = -NearPlane;

			BaseProjMatrix = FReversedZOrthoMatrix(
				HalfOrthoWidth,
				ScaledOrthoHeight,
				ZScale,
				ZOffset
			);
			ViewInitOptions.bUseFauxOrthoViewPos = true;

			// Added code, from FMinimalViewInfo::CalculateProjectionMatrix()
			const float Left = -1.0f + OffsetX;
			const float Right = Left + 2.0f;
			const float Bottom = -1.0f + OffsetY;
			const float Top = Bottom + 2.0f;
			BaseProjMatrix.M[2][0] = (Left + Right) / (Left - Right);
			BaseProjMatrix.M[2][1] = (Bottom + Top) / (Bottom - Top);

			// Modify the projection matrix to do an off center projection, with overlap for high-res tiling
			const bool bOrthographic = true;
			ModifyProjectionMatrixForTiling(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale);
		}
		else
		{
			float XAxisMultiplier;
			float YAxisMultiplier;

			if (CameraInfo.ViewInfo.bConstrainAspectRatio)
			{
				// If the camera's aspect ratio has a thinner width, then stretch the horizontal fov more than usual to 
				// account for the extra with of (before constraining - after constraining)
				if (CameraAspectRatio < DestAspectRatio)
				{
					const float ConstrainedWidth = ViewInitOptions.GetViewRect().Height() * CameraAspectRatio;
					XAxisMultiplier = ConstrainedWidth / (float)ViewInitOptions.GetViewRect().Width();
					YAxisMultiplier = CameraAspectRatio;
				}
				// Simplified some math here but effectively functions similarly to the above, the unsimplified code would look like:
				// const float ConstrainedHeight = ViewInitOptions.GetViewRect().Width() / CameraCache.AspectRatio;
				// YAxisMultiplier = (ConstrainedHeight / ViewInitOptions.GetViewRect.Height()) * CameraCache.AspectRatio;
				else
				{
					XAxisMultiplier = 1.0f;
					YAxisMultiplier = ViewInitOptions.GetViewRect().Width() / (float)ViewInitOptions.GetViewRect().Height();
				}
			}
			else
			{
				const int32 DestSizeX = ViewInitOptions.GetViewRect().Width();
				const int32 DestSizeY = ViewInitOptions.GetViewRect().Height();
				const EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULocalPlayer>()->AspectRatioAxisConstraint;
				if (((DestSizeX > DestSizeY) && (AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV)) || (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV))
				{
					//if the viewport is wider than it is tall
					XAxisMultiplier = 1.0f;
					YAxisMultiplier = ViewInitOptions.GetViewRect().Width() / (float)ViewInitOptions.GetViewRect().Height();
				}
				else
				{
					//if the viewport is taller than it is wide
					XAxisMultiplier = ViewInitOptions.GetViewRect().Height() / (float)ViewInitOptions.GetViewRect().Width();
					YAxisMultiplier = 1.0f;
				}
			}

			const float MinZ = CameraInfo.ViewInfo.GetFinalPerspectiveNearClipPlane();
			const float MaxZ = MinZ;
			// Avoid zero ViewFOV's which cause divide by zero's in projection matrix
			const float MatrixFOV = FMath::Max(0.001f, ViewFOV) * (float)PI / 360.0f;


			if ((bool)ERHIZBuffer::IsInverted)
			{
				BaseProjMatrix = FReversedZPerspectiveMatrix(
					MatrixFOV,
					MatrixFOV,
					XAxisMultiplier,
					YAxisMultiplier,
					MinZ,
					MaxZ
				);
			}
			else
			{
				BaseProjMatrix = FPerspectiveMatrix(
					MatrixFOV,
					MatrixFOV,
					XAxisMultiplier,
					YAxisMultiplier,
					MinZ,
					MaxZ
				);
			}

			// Added code, from FMinimalViewInfo::CalculateProjectionMatrix()
			const float Left = -1.0f + OffsetX;
			const float Right = Left + 2.0f;
			const float Bottom = -1.0f + OffsetY;
			const float Top = Bottom + 2.0f;
			BaseProjMatrix.M[2][0] = (Left + Right) / (Left - Right);
			BaseProjMatrix.M[2][1] = (Bottom + Top) / (Bottom - Top);

			// Modify the perspective matrix to do an off center projection, with overlap for high-res tiling
			const bool bOrthographic = false;
			ModifyProjectionMatrixForTiling(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale);
			// ToDo: Does orthographic support tiling in the same way or do I need to modify the values before creating the ortho view.
		}
	}
	// BaseProjMatrix may be perspective or orthographic.
	ViewInitOptions.ProjectionMatrix = BaseProjMatrix;

	ViewInitOptions.SceneViewStateInterface = GetSceneViewStateInterface(OptPayload);
	ViewInitOptions.FOV = ViewFOV;
	ViewInitOptions.DesiredFOV = ViewFOV;

	FSceneView* View = new FSceneView(ViewInitOptions);
	ViewFamily->Views.Add(View);


	View->ViewLocation = CameraInfo.ViewInfo.Location;
	View->ViewRotation = CameraInfo.ViewInfo.Rotation;
	// Override previous/current view transforms so that tiled renders don't use the wrong occlusion/motion blur information.
	View->PreviousViewTransform = CameraInfo.ViewInfo.PreviousViewTransform;

	View->StartFinalPostprocessSettings(View->ViewLocation);
	BlendPostProcessSettings(View, InOutSampleState, OptPayload);

	// Scaling sensor size inversely with the the projection matrix [0][0] should physically
	// cause the circle of confusion to be unchanged.
	View->FinalPostProcessSettings.DepthOfFieldSensorWidth *= DofSensorScale;
	// Modify the 'center' of the lens to be offset for high-res tiling, helps some effects (vignette) etc. still work.
	View->LensPrincipalPointOffsetScale = (FVector4f)CalculatePrinciplePointOffsetForTiling(InOutSampleState); // LWC_TODO: precision loss. CalculatePrinciplePointOffsetForTiling() could return float, it's normalized?
	View->EndFinalPostprocessSettings(ViewInitOptions);

	// This metadata is per-file and not per-view, but we need the blended result from the view to actually match what we rendered.
	// To solve this, we'll insert metadata per renderpass, separated by render pass name.
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/fstop"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFstop));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/fov"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(ViewInitOptions.FOV));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/focalDistance"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFocalDistance));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/sensorWidth"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldSensorWidth));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/overscanPercent"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(InOutSampleState.OverscanPercentage));

	InOutSampleState.OutputState.FileMetadata.Append(CameraInfo.FileMetadata);
	return View;
}

void UEArchVizMovieDeferredPass_PathTracer::ValidateStateImpl()
{
	Super::ValidateStateImpl();
	PathTracerValidationImpl();
}

void UEArchVizMovieDeferredPass_PathTracer::SetupImpl(const MoviePipeline::FMoviePipelineRenderPassInitSettings& InPassInitSettings)
{
	if (!CheckIfPathTracerIsSupported())
	{
		UE_LOG(LogMovieRenderPipeline, Error, TEXT("Cannot render a Path Tracer pass, Path Tracer is not enabled by this project."));
		GetPipeline()->Shutdown(true);
		return;
	}

	Super::SetupImpl(InPassInitSettings);
}
