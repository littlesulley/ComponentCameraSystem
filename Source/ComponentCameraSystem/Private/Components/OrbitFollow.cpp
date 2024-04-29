// Copyright 2023 by Sulley. All Rights Reserved.


#include "Components/OrbitFollow.h"
#include "Components/ControlAim.h"
#include "Components/SimpleFollow.h"
#include "Core/ECameraBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UOrbitFollow::UOrbitFollow()
{
	Stage = EStage::PostAim;

	BlendFunction = EEasingFunc::Linear;
	DampParams = FDampParams();
	
	DeltaResidual = FVector(0.0f, 0.0f, 0.0f);
	PreviousResidual = FVector(0, 0, 0);
	PreviousLocation = FVector(0, 0, 0);
	ExactSpringVel = FVector(0.0f, 0.0f, 0.0f);

	CurrentRootPosition = FVector(0, 0, 0);
	CachedRootPosition = FVector(0, 0, 0);
	CachedPitch = 0.0f;
	Height = 0.0f;
	Radius = 0.0f;
}

void UOrbitFollow::UpdateComponent_Implementation(float DeltaTime)
{
	if (FollowTarget != nullptr)
	{
		/** Do nothing if no orbit is assigned. */
		if (Orbits.Num() == 0) return;

		/** Follow position to track. */
		FVector FollowPosition = FollowTarget->GetActorLocation();

		/** Transform into camera's local space. */
		FVector LocalSpaceFollowPosition = UECameraLibrary::GetLocalSpacePositionWithVectors(CurrentRootPosition, GetOwningActor()->GetActorForwardVector(), GetOwningActor()->GetActorRightVector(), GetOwningActor()->GetActorUpVector(), FollowPosition);

		/** Temporary (before damping) delta position. */
		FVector TempDeltaPosition = LocalSpaceFollowPosition;

		/** Get damped delta position. */
		FVector SpringTemporalInput = LocalSpaceFollowPosition - PreviousLocation;
		FVector DampedDeltaPosition = DampDeltaPosition(TempDeltaPosition, SpringTemporalInput, DeltaTime);

		/** Transform DampedDeltaPosition from local space to world space.  */
		DampedDeltaPosition = UKismetMathLibrary::TransformDirection(FTransform(GetOwningActor()->GetActorRotation(), CurrentRootPosition, FVector::OneVector), DampedDeltaPosition);

		/** Update cache root position and current root position. */
		CachedRootPosition = CurrentRootPosition;
		CurrentRootPosition += DampedDeltaPosition;

		/** Update previous location. */
		PreviousLocation = UECameraLibrary::GetLocalSpacePositionWithVectors(CurrentRootPosition, GetOwningActor()->GetActorForwardVector(), GetOwningActor()->GetActorRightVector(), GetOwningActor()->GetActorUpVector(), FollowPosition);

		/** If the aim component is not ControlAim, only follows the target. */
		UControlAim* ControlAimComponent = Cast<UControlAim>(GetOwningSettingComponent()->GetAimComponent());
		if (!IsValid(ControlAimComponent))
		{
			GetOwningActor()->SetActorLocation(CurrentRootPosition);
			return;
		}

		/** Else, update height. */
		UpdateHeight(ControlAimComponent);

		/** Evaluate radius. */
		EvaluateRadius();

		/** Get world space orbit offset. */
		FVector OrbitOffset = GetOrbitOffset();

		/** Apply orbit offset. */
		GetOwningActor()->SetActorLocation(CurrentRootPosition + OrbitOffset);

		/** Calibrate camera orientation. */
		FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetOwningActor()->GetActorLocation(), CurrentRootPosition);
		GetOwningActor()->SetActorRotation(FRotator(DesiredRotation.Pitch, GetOwningActor()->GetActorRotation().Yaw, 0));
	}
}

void UOrbitFollow::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	if (FollowTarget == nullptr || Orbits.Num() == 0) return;

	/** If bPreserveState is true. */
	if (bPreserveState)
	{
		/** Reset Height to the place nearest to current camera's height,  */
		FVector FollowPosition = FollowTarget->GetActorLocation();
		float CurrentHeight = PC->PlayerCameraManager->GetCameraLocation().Z - FollowPosition.Z;
		Height = CurrentHeight;
		Height = ConstrainHeight(Height);

	}
	/** Else, reset Height to the middle. */
	else
	{
		Height = (Orbits[0].Height + Orbits[Orbits.Num() - 1].Height) / 2;
	}
	/** Reset CurrentRootPosition and CachedRootPosition for stable camera transition. */
	CurrentRootPosition = CachedRootPosition = FollowTarget->GetActorLocation();

	DeltaResidual = FVector(0, 0, 0);
	PreviousResidual = FVector(0, 0, 0);
	PreviousLocation = FVector(0, 0, 0);
	ExactSpringVel = FVector(0.0f, 0.0f, 0.0f);
}

FVector UOrbitFollow::DampDeltaPosition(const FVector& TempDeltaPosition, const FVector& SpringTemporalInput, float DeltaTime)
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

	return DampedDeltaPosition;
}

void UOrbitFollow::UpdateHeight(UControlAim* ControlAimComponent)
{
	float DeltaY = ControlAimComponent->GetMouseDeltaY();
	Height -= DeltaY;
	Height = ConstrainHeight(Height);
}

float UOrbitFollow::ConstrainHeight(float InputHeight)
{
	float OutputHeight = InputHeight;
	float UpperBoundHeight = Orbits[0].Height;
	float LowerBoundHeight = Orbits[Orbits.Num() - 1].Height;
	if (InputHeight > UpperBoundHeight) OutputHeight = UpperBoundHeight;
	if (InputHeight < LowerBoundHeight) OutputHeight = LowerBoundHeight;
	return OutputHeight;
}

void UOrbitFollow::EvaluateRadius()
{
	if (Orbits.Num() == 1)
	{
		Radius = Orbits[0].Radius;
		return;
	}

	FOrbit OrbitTop, OrbitBottom;
	for (int index = 0; index < Orbits.Num() - 1; ++index)
		if (Height <= Orbits[index].Height && Height >= Orbits[index + 1].Height)
		{
			OrbitTop = Orbits[index];
			OrbitBottom = Orbits[index + 1];
			break;
		}

	Radius = UKismetMathLibrary::Ease(OrbitBottom.Radius, OrbitTop.Radius, (Height - OrbitBottom.Height) / (OrbitTop.Height - OrbitBottom.Height), BlendFunction);
}

FVector UOrbitFollow::GetOrbitOffset()
{
	FVector Direction = GetOwningActor()->GetActorForwardVector();
	Direction.Z = 0.0f;
	Direction.Normalize();

	return Height * FVector::UpVector + Radius * (-Direction);
}