// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ModifyAimPointExtension.h"
#include "Components/FramingFollow.h"
#include "Components/TargetingAim.h"
#include "Kismet/KismetMathLibrary.h"


UModifyAimPointExtension::UModifyAimPointExtension()
{
	Stage = EStage::PostFollow;

	Radius = 500.f;
	Weights = FVector(0.2f, 0.5f, 0.3f);
	bInModify = false;
}

void UModifyAimPointExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (IsValid(ScreenFollowComponent) &&
		IsValid(TargetingAimComponent) &&
		IsValid(ScreenFollowComponent->GetFollowTarget()) &&
		IsValid(TargetingAimComponent->GetAimTarget()))
	{
		FVector FollowPosition = ScreenFollowComponent->GetFollowPosition();
		FVector AimPosition = TargetingAimComponent->GetAimPosition();
		float PlanarDistance = UKismetMathLibrary::Vector_Distance2D(FollowPosition, AimPosition);

		if (PlanarDistance < Radius)
		{
			FVector PitchAddition = FVector::ZeroVector;
			FVector CamToAimAddition = FVector::ZeroVector;
			FVector CamForwardAddition = FVector::ZeroVector;

			// Begin PitchAddition
			if (!bInModify)
			{
				bInModify = true;
				CachedPitch = UKismetMathLibrary::FindLookAtRotation(FollowPosition, AimPosition).Pitch;
			}

			FVector FollowToAim = AimPosition - FollowPosition;
			FVector Projected = UKismetMathLibrary::ProjectVectorOnToPlane(FollowToAim, FVector(0, 0, 1));
			FVector Vertical = FVector(0, 0, 1) * Projected.Size() * UKismetMathLibrary::DegTan(CachedPitch);
			FVector FixedDirection = Projected + Vertical;

			FixedDirection.Normalize();
			FixedDirection *= Radius / UKismetMathLibrary::DegCos(CachedPitch);
			PitchAddition = FixedDirection - FollowToAim;

			// Begin CamToAimAddition
			float Magnitude = 0.f;

			FVector FollowToCam = -(FollowPosition - GetOwningActor()->GetActorLocation());
			FVector CamToAim = AimPosition - GetOwningActor()->GetActorLocation();

			float CurrentLength = CamToAim.Size();
			CamToAim.Normalize();

			float A = CamToAim.X * CamToAim.X + CamToAim.Y * CamToAim.Y;
			float B = 2.0 * (FollowToCam.X * CamToAim.X + FollowToCam.Y * CamToAim.Y);
			float C = FollowToCam.X * FollowToCam.X + FollowToCam.Y * FollowToCam.Y - Radius * Radius;
			float Delta = B * B - 4.0 * A * C;

			if (Delta > 0)
			{
				Magnitude = (-B + FMath::Sqrt(Delta)) / (2.0 * A) - CurrentLength;
			}

			CamToAimAddition = CamToAim * Magnitude;

			// Begin CamForwardAddition
			FVector CamDir = UKismetMathLibrary::ProjectVectorOnToPlane(GetOwningActor()->GetActorForwardVector(), FVector(0, 0, 1));
			CamDir.Normalize();

			A = CamDir.X * CamDir.X + CamDir.Y * CamDir.Y;
			B = 2.0 * (FollowToAim.X * CamDir.X + FollowToAim.Y * CamDir.Y);
			C = FollowToAim.X * FollowToAim.X + FollowToAim.Y * FollowToAim.Y - Radius * Radius;
			Delta = B * B - 4.0 * A * C;

			if (Delta > 0)
			{
				Magnitude = (-B + FMath::Sqrt(Delta)) / (2.0 * A);
			}

			CamForwardAddition = CamDir * Magnitude;

			// Blend between the two types of additions
			FVector Addition = Weights[0] * PitchAddition + Weights[1] * CamToAimAddition + Weights[2] * CamForwardAddition;
			TargetingAimComponent->SetAdditionalAimOffset(Addition);
		}
		else
		{
			bInModify = false;
			TargetingAimComponent->SetAdditionalAimOffset();
		}
	}
}

void UModifyAimPointExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	ScreenFollowComponent = Cast<UFramingFollow>(OwningSettingComponent->GetFollowComponent());
	TargetingAimComponent = Cast<UTargetingAim>(OwningSettingComponent->GetAimComponent());
}