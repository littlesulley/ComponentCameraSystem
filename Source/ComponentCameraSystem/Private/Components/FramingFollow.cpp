// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/FramingFollow.h"
#include "Core/ECameraSettingsComponent.h"
#include "Components/ECameraComponentAim.h"
#include "Components/TargetingAim.h"
#include "Components/ControlAim.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputActionValue.h"

UFramingFollow::UFramingFollow()
{
	Stage = EStage::PostAim;

	CameraDistance = 200.0f;
	FollowOffset = FVector(0.0f, 0.0f, 0.0f);
	bAdaptToMovement = false;
	AdaptToMovementSpeed = 1.0f;
	ZoomSettings = FZoomSettings();
	DampParams = FDampParams();
	ScreenOffset = FVector2D(0.0f, 0.0f);
	AdaptiveScreenOffsetDistanceX = FVector2D(200.0f, -100.0f);
	AdaptiveScreenOffsetDistanceY = FVector2D(200.0f, -100.0f);
	ScreenOffsetWidth = FVector2D(-0.1f, 0.1f);
	ScreenOffsetHeight = FVector2D(-0.1f, 0.1f);
	DeltaResidual = FVector(0.0f, 0.0f, 0.0f);
	PreviousResidual = FVector(0.0f, 0.0f, 0.0f);
	PreviousLocation = FVector(0.0f, 0.0f, 0.0f);
	ExactSpringVel = FVector(0.0f, 0.0f, 0.0f);
	CachedZoomValue = 0.0f;
}

void UFramingFollow::UpdateComponent_Implementation(float DeltaTime)
{
	if (FollowTarget != nullptr)
	{
		FVector FollowPosition = UECameraLibrary::GetPositionWithLocalOffset(FollowTarget.Get(), FollowOffset);

		/** Get real screen offset. */
		FVector AimPosition = FVector(0, 0, 0);
		FVector2D RealScreenOffset = FVector2D(0, 0);
		if (GetOwningSettingComponent()->GetAimComponent() != nullptr &&
			GetOwningSettingComponent()->GetAimComponent()->IsA<class UTargetingAim>() &&
			GetOwningSettingComponent()->GetAimComponent()->GetAimTarget() != nullptr)
		{
			AimPosition = GetOwningSettingComponent()->GetAimComponent()->GetRealAimPosition();
			RealScreenOffset = GetAdaptiveScreenOffset(FollowPosition, AimPosition);
		}
		else RealScreenOffset = ScreenOffset;

		/** Transform from world space to local space. */
		FVector LocalSpaceFollowPosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), FollowPosition);

		/** Temporary (before damping) delta position. */
		FVector TempDeltaPosition = FVector(0, 0, 0);

		/** If using float curve, get the adaptive camera distance. */
		float AdaptiveCameraDistance = CameraDistance;
		if (PitchDistanceCurve != nullptr)
		{
			float Pitch = NormalizePitch(GetOwningActor()->GetActorRotation().Pitch);
			AdaptiveCameraDistance = PitchDistanceCurve->GetFloatValue(Pitch);
		}

		/** If using zooming, override the default camera distance. */
		if (ZoomSettings.bEnableZoom && ZoomSettings.ZoomAction && Subsystem)
		{
			/** Whether CurrentCameraDistance has been initialized. */
			if (CurrentCameraDistance == -1.0f)
			{
				CurrentCameraDistance = CameraDistance;
			}

			UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
			if (PlayerInput)
			{
				FInputActionValue Value = PlayerInput->GetActionValue(ZoomSettings.ZoomAction);
				float ZoomValue = Value.Get<float>();
				CachedZoomValue = CachedZoomValue + GetDampedZoomValue(ZoomValue, DeltaTime);

				float CameraDistanceZoomGain = ZoomSettings.Speed * 10.f * DeltaTime * CachedZoomValue;
				CurrentCameraDistance = FMath::Clamp(CurrentCameraDistance + CameraDistanceZoomGain, ZoomSettings.DistanceBounds.X, ZoomSettings.DistanceBounds.Y);
			}
		}
		else
		{
			CurrentCameraDistance = AdaptiveCameraDistance;
		}

		/** First move the camera along the local space X axis. */
		SetForwardDelta(LocalSpaceFollowPosition, TempDeltaPosition, CurrentCameraDistance);

		/** Then move the camera along the local space YZ plane. */
		SetYZPlaneDelta(LocalSpaceFollowPosition, TempDeltaPosition, RealScreenOffset, CurrentCameraDistance);

		/** Get damped delta position. */
		FVector SpringTemporalInput = (LocalSpaceFollowPosition - FVector(CurrentCameraDistance, 0, 0)) - PreviousLocation;
		FVector DampedDeltaPosition = DampDeltaPosition(LocalSpaceFollowPosition, SpringTemporalInput, TempDeltaPosition, DeltaTime, RealScreenOffset);

		/** Apply damped delta position. */
		GetOwningActor()->AddActorLocalOffset(DampedDeltaPosition);

		/** Cache current position, in local space. */
		LocalSpaceFollowPosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), FollowPosition);
		PreviousLocation = LocalSpaceFollowPosition - FVector(CurrentCameraDistance, 0, 0);

		/** Check if adapting to movement. */
		if (bAdaptToMovement && !HasControlAimInput())
		{			
			FVector LocalVelocity = UKismetMathLibrary::InverseTransformDirection(GetOwningActor()->GetActorTransform(), FollowTarget->GetVelocity());
			GetOwningActor()->AddActorWorldRotation(FRotator(0, AdaptToMovementSpeed * LocalVelocity.Y / 1000.0f, 0));
			
			FVector RectifiedFollowPosition = UKismetMathLibrary::TransformDirection(GetOwningActor()->GetActorTransform(), LocalSpaceFollowPosition) + GetOwningActor()->GetActorLocation();
			GetOwningActor()->AddActorWorldOffset(FollowPosition - RectifiedFollowPosition);
		}

		/** Update variables for HUD. */
		RealFollowPosition = FollowPosition;
		LocalFollowPosition = LocalSpaceFollowPosition;
	}
}

void UFramingFollow::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	if (GetOwningSettingComponent() && GetOwningSettingComponent()->GetAimComponent() && GetOwningSettingComponent()->GetAimComponent()->IsA(UControlAim::StaticClass()))
	{
		ControlAim = Cast<UControlAim>(GetOwningSettingComponent()->GetAimComponent());
	}

	DeltaResidual = FVector(0, 0, 0);
	PreviousResidual = FVector(0.0f, 0.0f, 0.0f);
	PreviousLocation = FVector(0.0f, 0.0f, 0.0f);
	ExactSpringVel = FVector(0.0f, 0.0f, 0.0f);
	CachedZoomValue = 0.0f;

	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningSettingComponent()->GetPlayerController()->GetLocalPlayer());
}

FVector2D UFramingFollow::GetAdaptiveScreenOffset(const FVector& FollowPosition, const FVector& AimPosition)
{
	FVector Diff = FollowPosition - AimPosition;
	float ProjectedDistance = FMath::Sqrt(FMath::Square(Diff.X) + FMath::Square(Diff.Y));

	FVector2D OutRange = FVector2D(1.0f, 0.0f);
	FVector2D RealScreenOffset;
	RealScreenOffset.X = ScreenOffset.X * FMath::GetMappedRangeValueClamped(AdaptiveScreenOffsetDistanceX, OutRange, ProjectedDistance);
	RealScreenOffset.Y = ScreenOffset.Y * FMath::GetMappedRangeValueClamped(AdaptiveScreenOffsetDistanceY, OutRange, ProjectedDistance);

	return RealScreenOffset;
}

float UFramingFollow::NormalizePitch(float Pitch)
{
	float NormalizedPitch = Pitch;
	while (Pitch > 90) Pitch -= 360;
	while (Pitch < -90) Pitch += 360;
	return NormalizedPitch;
}

void UFramingFollow::SetForwardDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, float RealCameraDistance)
{
	TempDeltaPosition.X = LocalSpaceFollowPosition.X - RealCameraDistance;
}

void UFramingFollow::SetYZPlaneDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, const FVector2D& RealScreenOffset, float RealCamemraDistance)
{
	/** Note that CameraDistance is used assuming that the target is already at the CameraDistance place. */
	float W = UKismetMathLibrary::DegTan(OwningCamera->GetCameraComponent()->FieldOfView / 2.0f) * RealCamemraDistance * 2.0f;
	float ExpectedPositionY = W * RealScreenOffset.X;
	float ExpectedPositionZ = W / OwningCamera->GetCameraComponent()->AspectRatio * RealScreenOffset.Y;

	TempDeltaPosition.Y = LocalSpaceFollowPosition.Y - ExpectedPositionY;
	TempDeltaPosition.Z = LocalSpaceFollowPosition.Z - ExpectedPositionZ;
}

FVector UFramingFollow::DampDeltaPosition(const FVector& LocalSpaceFollowPosition, const FVector& SpringTemporalInput, const FVector& TempDeltaPosition, float DeltaTime, const FVector2D& RealScreenOffset)
{
	FVector DampedDeltaPosition = FVector(0, 0, 0);
	
	UECameraLibrary::EasyDampVectorWithDifferentDampTime(
		DampParams,
		TempDeltaPosition,
		DeltaTime,
		DampParams.DampTime,
		DampedDeltaPosition,
		SpringTemporalInput,
		TempDeltaPosition,
		ExactSpringVel,
		TempDeltaPosition,
		UKismetMathLibrary::InverseTransformDirection(GetOwningActor()->GetActorTransform(), FollowTarget->GetVelocity()) / 1.1f,
		ExactSpringVel,
		PreviousResidual,
		DeltaResidual
	);

	PreviousResidual = TempDeltaPosition - DampedDeltaPosition;

	EnsureWithinBounds(LocalSpaceFollowPosition, DampedDeltaPosition, RealScreenOffset, CurrentCameraDistance);

	return DampedDeltaPosition;
}

void UFramingFollow::EnsureWithinBounds(const FVector& LocalSpaceFollowPosition, FVector& DampedDeltaPosition, const FVector2D& RealScreenOffset, float RealCamemraDistance)
{
	FVector ResultLocalSpacePosition = LocalSpaceFollowPosition - DampedDeltaPosition;

	/** Same as before. Use CameraDistance as the ground-truth distance. */
	float Width = UKismetMathLibrary::DegTan(OwningCamera->GetCameraComponent()->FieldOfView / 2.0f) * RealCamemraDistance * 2.0f;
	float LeftBound = (RealScreenOffset.X + ScreenOffsetWidth.X) * Width;
	float RightBound = (RealScreenOffset.X + ScreenOffsetWidth.Y) * Width;
	float BottomBound = (RealScreenOffset.Y + ScreenOffsetHeight.X) * Width / OwningCamera->GetCameraComponent()->AspectRatio;
	float TopBound = (RealScreenOffset.Y + ScreenOffsetHeight.Y) * Width / OwningCamera->GetCameraComponent()->AspectRatio;
	
	if (ResultLocalSpacePosition.Y < LeftBound)   DampedDeltaPosition.Y += ResultLocalSpacePosition.Y - LeftBound;
	if (ResultLocalSpacePosition.Y > RightBound)  DampedDeltaPosition.Y += ResultLocalSpacePosition.Y - RightBound;
	if (ResultLocalSpacePosition.Z < BottomBound) DampedDeltaPosition.Z += ResultLocalSpacePosition.Z - BottomBound;
	if (ResultLocalSpacePosition.Z > TopBound)    DampedDeltaPosition.Z += ResultLocalSpacePosition.Z - TopBound;
}

bool UFramingFollow::HasControlAimInput()
{
	if (ControlAim && ControlAim->GetMouseDeltaX() != 0.0f)
	{
		return true;
	}
	
	return false;
}

float UFramingFollow::GetDampedZoomValue(const float& ZoomValue, const float& DeltaTime)
{
	float Output = 0;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, ZoomValue - CachedZoomValue, ZoomSettings.DampTime, Output);
	return Output;
}