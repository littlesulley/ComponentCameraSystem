// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ModifyAimPointExtension.h"
#include "Components/FramingFollow.h"
#include "Components/TargetingAim.h"
#include "Kismet/KismetMathLibrary.h"


UModifyAimPointExtension::UModifyAimPointExtension()
{
	Stage = EStage::PostFollow;

	Radius = 500.f;
	Strength = 0.95f;
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
		FVector AimPosition = TargetingAimComponent->GetOriginalAimPosition();
		float PlanarDistance = UKismetMathLibrary::Vector_Distance2D(FollowPosition, AimPosition);

		if (PlanarDistance < Radius)
		{	
			// Begin fixed addition: trying to keep a fixed angle
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
			FVector AdditionOfFixed = FixedDirection - FollowToAim;

			// Begin dynamic addition: trying to keep the aim position at a static screen space position
			FVector FollowToCam = -(FollowPosition - GetOwningActor()->GetActorLocation());
			FVector CamToAim = AimPosition - GetOwningActor()->GetActorLocation();

			float CurrentLength = CamToAim.Size();
			float TargetLength = CurrentLength;
			CamToAim.Normalize();

			float A = 1.0;
			float B = 2.0 * FVector::DotProduct(FollowToCam, CamToAim);
			float C = FollowToCam.SizeSquared() - Radius * Radius;
			float Delta = B * B - 4.0 * A * C;

			if (Delta > 0)
			{
				TargetLength = (-B + FMath::Sqrt(Delta)) / 2.0;
			}

			float Magnitude = TargetLength - CurrentLength;
			FVector AdditionOfDynamic = CamToAim * Magnitude;

			// Blend between the two types of additions
			FVector Addition = (1 - Strength) * AdditionOfFixed + Strength * AdditionOfDynamic;
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