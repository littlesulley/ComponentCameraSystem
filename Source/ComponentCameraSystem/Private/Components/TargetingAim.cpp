// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/TargetingAim.h"
#include "Core/ECameraSettingsComponent.h"
#include "Components/ECameraComponentAim.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"

UTargetingAim::UTargetingAim()
{
	Stage = EStage::Aim;

	AdditionalAimOffset = FVector(0.0f, 0.0f, 0.0f);
	Damper = CreateDefaultSubobject<UECameraRotatorDamper>("Camera Damper Vector");
	ScreenOffset = FVector2f(0.0f, 0.0f);
	ScreenOffsetWidth = FVector2f(-0.1f, 0.1f);
	ScreenOffsetHeight = FVector2f(-0.1f, 0.1f);
}

void UTargetingAim::UpdateComponent_Implementation(float DeltaTime)
{
	if (AimTarget != nullptr)
	{
		/** Get the *real* aim position, based on actor's local space. */
		FVector AimPosition = GetRealAimPosition(true);

		/** If camera is too close to aim target, return. */
		if (CheckIfTooClose(AimPosition)) return;

		/** Temporary delta rotation before damping. */
		FRotator TempDeltaRotation = FRotator(0, 0, 0);

		/** Set delta rotation. */
		SetDeltaRotation(AimPosition, TempDeltaRotation);

		/** Get damped delta rotation. */
		FRotator DampedDeltaRotation = DampDeltaRotation(TempDeltaRotation, DeltaTime, AimPosition);

		/** Apply damped delta rotation. */
		GetOwningActor()->AddActorLocalRotation(FRotator(DampedDeltaRotation.Pitch, 0, 0));
		GetOwningActor()->AddActorWorldRotation(FRotator(0, DampedDeltaRotation.Yaw, 0));
	}
}

bool UTargetingAim::CheckIfTooClose(const FVector& AimPosition)
{
	float Distance = UKismetMathLibrary::Vector_Distance(GetOwningActor()->GetActorLocation(), AimPosition);
	return UKismetMathLibrary::NearlyEqual_FloatFloat(Distance, 0, 0.001);
}

void UTargetingAim::SetDeltaRotation(const FVector& AimPosition, FRotator& TempDeltaRotation)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetOwningActor()->GetActorLocation(), AimPosition);
	FRotator CenteredDeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetOwningActor()->GetActorRotation());
		
	float TanHalfFOV = UKismetMathLibrary::DegTan(OwningCamera->GetCameraComponent()->FieldOfView / 2);
	TempDeltaRotation.Yaw = CenteredDeltaRotation.Yaw - UKismetMathLibrary::DegAtan(2.0 * ScreenOffset.X * TanHalfFOV);
	TempDeltaRotation.Pitch = CenteredDeltaRotation.Pitch - UKismetMathLibrary::DegAtan(2.0 * ScreenOffset.Y * TanHalfFOV / OwningCamera->GetCameraComponent()->AspectRatio);
	TempDeltaRotation.Roll = 0;
}

FRotator UTargetingAim::DampDeltaRotation(const FRotator& TempDeltaRotation, float DeltaTime, const FVector& AimPosition)
{
	FRotator DampedDeltaRotation = TempDeltaRotation;

	if (Damper)
	{
		Damper->SetInput(DampedDeltaRotation);
		DampedDeltaRotation = Damper->ApplyDamp(DeltaTime);
		EnsureWithinBounds(DampedDeltaRotation, AimPosition);
		Damper->SetOutput(DampedDeltaRotation);
		Damper->PostApplyDamp();
	}

	return DampedDeltaRotation;
}

void UTargetingAim::EnsureWithinBounds(FRotator& DampedDeltaRotation, const FVector& AimPosition)
{
	float TanHalfFOV = UKismetMathLibrary::DegTan(OwningCamera->GetCameraComponent()->FieldOfView / 2);
	double LeftBound = UKismetMathLibrary::DegAtan(2.0 * (ScreenOffset.X + ScreenOffsetWidth.X) * TanHalfFOV);
	double RightBound = UKismetMathLibrary::DegAtan(2.0 * (ScreenOffset.X + ScreenOffsetWidth.Y) * TanHalfFOV);
	double BottomBound = UKismetMathLibrary::DegAtan(2.0 * (ScreenOffset.Y + ScreenOffsetHeight.X) * TanHalfFOV / OwningCamera->GetCameraComponent()->AspectRatio);
	double TopBound = UKismetMathLibrary::DegAtan(2.0 * (ScreenOffset.Y + ScreenOffsetHeight.Y) * TanHalfFOV / OwningCamera->GetCameraComponent()->AspectRatio);

	FQuat DesiredQuat = GetOwningActor()->GetActorRotation().Quaternion();
	DesiredQuat = FQuat(FRotator(0, DampedDeltaRotation.Yaw, 0)) * DesiredQuat * FQuat(FRotator(DampedDeltaRotation.Pitch, 0, 0));
	FRotator DesiredRotation = DesiredQuat.Rotator();

	FRotator ResultRotationDiff = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(GetOwningActor()->GetActorLocation(), AimPosition), DesiredRotation);
	if (ResultRotationDiff.Yaw < LeftBound) DampedDeltaRotation.Yaw += ResultRotationDiff.Yaw - LeftBound;
	if (ResultRotationDiff.Yaw > RightBound) DampedDeltaRotation.Yaw += ResultRotationDiff.Yaw - RightBound;
	if (ResultRotationDiff.Pitch < BottomBound) DampedDeltaRotation.Pitch += ResultRotationDiff.Pitch - BottomBound;
	if (ResultRotationDiff.Pitch > TopBound) DampedDeltaRotation.Pitch += ResultRotationDiff.Pitch - TopBound;
}