// Copyright 2023 by Sulley. All Rights Reserved.

#include "EArchVizMovieDeferredPasses.h"
#include "MoviePipeline.h"
#include "Engine/LocalPlayer.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"

FSceneView* UEArchVizMovieDeferredPass::GetSceneViewForSampleState(FSceneViewFamily* ViewFamily, FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload)
{
	APlayerController* LocalPlayerController = GetPipeline()->GetWorld()->GetFirstPlayerController();

	APlayerCameraManager* PlayerCameraManager = GetPipeline()->GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	FMinimalViewInfo POV = PlayerCameraManager->GetCameraCachePOV();

	double OffsetX = POV.OffCenterProjectionOffset.X;
	double OffsetY = POV.OffCenterProjectionOffset.Y;

	int32 TileSizeX = InOutSampleState.BackbufferSize.X;
	int32 TileSizeY = InOutSampleState.BackbufferSize.Y;

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.ViewOrigin = InOutSampleState.FrameInfo.CurrViewLocation;
	ViewInitOptions.SetViewRectangle(FIntRect(FIntPoint(0, 0), FIntPoint(TileSizeX, TileSizeY)));
	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(InOutSampleState.FrameInfo.CurrViewRotation);
	ViewInitOptions.ViewActor = LocalPlayerController ? LocalPlayerController->GetViewTarget() : nullptr;

	// Rotate the view 90 degrees (reason: unknown)
	ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	float ViewFOV = 90.f;
	if (GetPipeline()->GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
	{
		ViewFOV = GetPipeline()->GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetFOVAngle();
	}

	// Inflate our FOV to support the overscan 
	ViewFOV = 2.0f * FMath::RadiansToDegrees(FMath::Atan((1.0f + InOutSampleState.OverscanPercentage) * FMath::Tan(FMath::DegreesToRadians(ViewFOV * 0.5f))));

	float DofSensorScale = 1.0f;

	// Calculate a Projection Matrix
	{
		float XAxisMultiplier;
		float YAxisMultiplier;

		check(GetPipeline()->GetWorld());
		check(GetPipeline()->GetWorld()->GetFirstPlayerController());

		// Stretch the fovs if the view is constrained to the camera's aspect ratio
		if (PlayerCameraManager && PlayerCameraManager->GetCameraCachePOV().bConstrainAspectRatio)
		{
			const FMinimalViewInfo CameraCache = PlayerCameraManager->GetCameraCachePOV();
			const float DestAspectRatio = ViewInitOptions.GetViewRect().Width() / (float)ViewInitOptions.GetViewRect().Height();

			// If the camera's aspect ratio has a thinner width, then stretch the horizontal fov more than usual to 
			// account for the extra with of (before constraining - after constraining)
			if (CameraCache.AspectRatio < DestAspectRatio)
			{
				const float ConstrainedWidth = ViewInitOptions.GetViewRect().Height() * CameraCache.AspectRatio;
				XAxisMultiplier = ConstrainedWidth / (float)ViewInitOptions.GetViewRect().Width();
				YAxisMultiplier = CameraCache.AspectRatio;
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

		const float MinZ = GNearClippingPlane;
		const float MaxZ = MinZ;
		// Avoid zero ViewFOV's which cause divide by zero's in projection matrix
		const float MatrixFOV = FMath::Max(0.001f, ViewFOV) * (float)PI / 360.0f;

		FMatrix BaseProjMatrix;

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
		ModifyProjectionMatrixForTiling(InOutSampleState, /*InOut*/ BaseProjMatrix, DofSensorScale);
		ViewInitOptions.ProjectionMatrix = BaseProjMatrix;
	}

	ViewInitOptions.SceneViewStateInterface = GetSceneViewStateInterface(OptPayload);
	ViewInitOptions.FOV = ViewFOV;

	FSceneView* View = new FSceneView(ViewInitOptions);
	ViewFamily->Views.Add(View);
	View->ViewLocation = InOutSampleState.FrameInfo.CurrViewLocation;
	View->ViewRotation = InOutSampleState.FrameInfo.CurrViewRotation;
	// Override previous/current view transforms so that tiled renders don't use the wrong occlusion/motion blur information.
	View->PreviousViewTransform = FTransform(InOutSampleState.FrameInfo.PrevViewRotation, InOutSampleState.FrameInfo.PrevViewLocation);

	View->StartFinalPostprocessSettings(View->ViewLocation);
	BlendPostProcessSettings(View);

	// Scaling sensor size inversely with the the projection matrix [0][0] should physically
	// cause the circle of confusion to be unchanged.
	View->FinalPostProcessSettings.DepthOfFieldSensorWidth *= DofSensorScale;
	// Modify the 'center' of the lens to be offset for high-res tiling, helps some effects (vignette) etc. still work.
	View->LensPrincipalPointOffsetScale = (FVector4f)CalculatePrinciplePointOffsetForTiling(InOutSampleState); // LWC_TODO: precision loss. CalculatePrinciplePointOffsetForTiling() could return float, it's normalized?
	View->EndFinalPostprocessSettings(ViewInitOptions);

	// This metadata is per-file and not per-view, but we need the blended result from the view to actually match what we rendered.
	// To solve this, we'll insert metadata per renderpass, separated by render pass name.
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/fstop"), *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFstop));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/fov"), *PassIdentifier.Name), FString::SanitizeFloat(ViewInitOptions.FOV));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/focalDistance"), *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFocalDistance));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/sensorWidth"), *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldSensorWidth));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/overscanPercent"), *PassIdentifier.Name), FString::SanitizeFloat(InOutSampleState.OverscanPercentage));

	if (GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
	{
		// This only works if you use a Cine Camera (which is almost guranteed with Sequencer) and it's easier (and less human error prone) than re-deriving the information
		ACineCameraActor* CineCameraActor = Cast<ACineCameraActor>(GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetViewTarget());
		if (CineCameraActor)
		{
			UCineCameraComponent* CineCameraComponent = CineCameraActor->GetCineCameraComponent();
			if (CineCameraComponent)
			{
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/sensorWidth"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->Filmback.SensorWidth));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/sensorHeight"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->Filmback.SensorHeight));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/sensorAspectRatio"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->Filmback.SensorAspectRatio));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/minFocalLength"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->LensSettings.MinFocalLength));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/maxFocalLength"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->LensSettings.MaxFocalLength));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/minFStop"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->LensSettings.MinFStop));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/maxFStop"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->LensSettings.MaxFStop));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/dofDiaphragmBladeCount"), *PassIdentifier.Name), FString::FromInt(CineCameraComponent->LensSettings.DiaphragmBladeCount));
				InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/camera/%s/focalLength"), *PassIdentifier.Name), FString::SanitizeFloat(CineCameraComponent->CurrentFocalLength));
			}
		}
	}


	return View;
}