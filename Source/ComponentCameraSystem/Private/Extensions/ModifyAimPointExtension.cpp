// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ModifyAimPointExtension.h"
#include "Components/FramingFollow.h"
#include "Components/TargetingAim.h"
#include "Kismet/KismetMathLibrary.h"


UModifyAimPointExtension::UModifyAimPointExtension()
{
	Stage = EStage::PreFollow;

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
				FixedAngle = UKismetMathLibrary::FindLookAtRotation(ScreenFollowComponent->GetFollowPosition(), TargetingAimComponent->GetOriginalAimPosition()).Pitch;
			}

			FVector FollowToAim = TargetingAimComponent->GetOriginalAimPosition() - ScreenFollowComponent->GetFollowPosition();
			FVector Projected = UKismetMathLibrary::ProjectVectorOnToPlane(FollowToAim, FVector(0, 0, 1));
			FVector Vertical = FVector(0, 0, 1) * Projected.Size() * UKismetMathLibrary::DegTan(FixedAngle);
			FVector FixedDirection = Projected + Vertical;

			FixedDirection.Normalize();
			FixedDirection *= Radius / UKismetMathLibrary::DegCos(FixedAngle);
			FVector AdditionOfFixed = FixedDirection - FollowToAim;

			// Begin dynamic addition: trying to keep the aim position at a static screen space position
			FVector CamToAim = TargetingAimComponent->GetRealAimPosition() - GetOwningActor()->GetActorLocation();
			CamToAim.Normalize();

			Projected = UKismetMathLibrary::ProjectVectorOnToPlane(CamToAim, FVector(0, 0, 1));
			Projected.Normalize();
			float Magnitude = (Radius - PlanarDistance) / ((Projected | CamToAim) + 0.0001);

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