// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ResolveGroupActorExtension.h"
#include "Utils/ECameraGroupActor.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraGroupActorComponent.h"
#include "Kismet/KismetMathLibrary.h"

UResolveGroupActorExtension::UResolveGroupActorExtension()
{
	Stage = EStage::PostAim;
	ResolveMethod = EResolveGroupActorMethod::DistanceOnly;
	FOVRange = FVector2f(90.0f, 120.0f);
	FOVDampTime = 1.0f;
	DistanceRange = FVector2f(-1200.0f, -400.0f);
	DistanceDampTime = 1.0f;
	Tolerance = 0.3f;
	AdjustedDistance = 0.0f;
}

void UResolveGroupActorExtension::UpdateComponent_Implementation(float DeltaTime)
{
	AECameraGroupActor* FollowGroupActor = Cast<AECameraGroupActor>(GetOwningSettingComponent()->GetFollowTarget());
	AECameraGroupActor* AimGroupActor = Cast<AECameraGroupActor>(GetOwningSettingComponent()->GetAimTarget());

	TArray<FBoundingWrappedActor> TargetActorsRef;
	if (FollowGroupActor) TargetActorsRef.Append(FollowGroupActor->CameraGroupActorComponent->TargetActors);
	if (AimGroupActor) TargetActorsRef.Append(AimGroupActor->CameraGroupActorComponent->TargetActors);
	if (TargetActorsRef.Num() != 0) ResolveGroupActor(TargetActorsRef, DeltaTime);
}

void UResolveGroupActorExtension::ResolveGroupActor(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime)
{
	if (ResolveMethod == EResolveGroupActorMethod::ZoomOnly)
	{
		float ResultDeltaFOV = GetFOVForZoomOnly(TargetActors, DeltaTime);
		GetCameraComponent()->FieldOfView += ResultDeltaFOV;
		return;
	}
	else if (ResolveMethod == EResolveGroupActorMethod::DistanceOnly)
	{
		/** Should first apply already adjusted distance. */
		GetOwningActor()->AddActorLocalOffset(FVector(AdjustedDistance, 0, 0));
		float ResultDeltaDistance = GetDistanceForDistanceOnly(TargetActors, DeltaTime, GetCameraComponent()->FieldOfView);
		/** Then apply delta distance. */
		GetOwningActor()->AddActorLocalOffset(FVector(ResultDeltaDistance, 0, 0));
		return;
	}
	else if (ResolveMethod == EResolveGroupActorMethod::Mix)
	{
		/** Apply already adjusted distance at the very beginning. */
		GetOwningActor()->AddActorLocalOffset(FVector(AdjustedDistance, 0, 0));

		/** Then, get and apply result delat FOV. */
		float ResultDeltaFOV = GetFOVForZoomOnly(TargetActors, DeltaTime);
		GetCameraComponent()->FieldOfView += ResultDeltaFOV;

		/** Next to check if FOV is nearly at the range bounds. If so, adjust distance. */
		if ((UKismetMathLibrary::NearlyEqual_FloatFloat(GetCameraComponent()->FieldOfView, FOVRange.Y, 1)
			|| UKismetMathLibrary::NearlyEqual_FloatFloat(GetCameraComponent()->FieldOfView, FOVRange.X, 1)))
		{
			float ResultDeltaDistance = GetDistanceForDistanceOnly(TargetActors, DeltaTime, GetCameraComponent()->FieldOfView);
			GetOwningActor()->AddActorLocalOffset(FVector(ResultDeltaDistance, 0, 0));
		}
		/** Else, try to first adjust distance back to zero. */
		else
		{
			if (!UKismetMathLibrary::NearlyEqual_FloatFloat(AdjustedDistance, 0, 1))
			{
				float ResultDeltaDistance = GetDistanceForDistanceOnly(TargetActors, DeltaTime, GetCameraComponent()->FieldOfView);
				GetOwningActor()->AddActorLocalOffset(FVector(ResultDeltaDistance, 0, 0));
			}
		}
		return;
	}
}

float UResolveGroupActorExtension::GetFOVForZoomOnly(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime)
{
	/** Required FOV, a minimum value of DefaultFOV. */
	float RequiredFOV = FOVRange.X;
	for (FBoundingWrappedActor& BWActor : TargetActors)
	{
		FVector LocalSpacePosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), BWActor.Target->GetActorLocation());
		/** Only consider targets in front of camera. */
		if (LocalSpacePosition.X > 0 && !BWActor.bExcludeBoundingBox)
		{
			float RightBound = (LocalSpacePosition.Y + BWActor.Width) * (1 - Tolerance);
			float LeftBound = (LocalSpacePosition.Y - BWActor.Width) * (1 - Tolerance);
			float TopBound = (LocalSpacePosition.Z + BWActor.Height) * (1 - Tolerance);
			float BottomBound = (LocalSpacePosition.Z - BWActor.Height) * (1 - Tolerance);

			if (RightBound > 0) RequiredFOV = FMath::Max(RequiredFOV, 2 * UKismetMathLibrary::DegAtan(RightBound / LocalSpacePosition.X));
			if (LeftBound < 0) RequiredFOV = FMath::Max(RequiredFOV, 2 * UKismetMathLibrary::DegAtan(-LeftBound / LocalSpacePosition.X));
			if (TopBound > 0) RequiredFOV = FMath::Max(RequiredFOV, 2 * UKismetMathLibrary::DegAtan(TopBound / LocalSpacePosition.X * GetCameraComponent()->AspectRatio));
			if (BottomBound < 0) RequiredFOV = FMath::Max(RequiredFOV, 2 * UKismetMathLibrary::DegAtan(-BottomBound / LocalSpacePosition.X * GetCameraComponent()->AspectRatio));
		}
	}
	
	/** Damp FOV. */
	double ResultDeltaFOV;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, RequiredFOV - GetCameraComponent()->FieldOfView, FOVDampTime, ResultDeltaFOV);
	if (GetCameraComponent()->FieldOfView + ResultDeltaFOV > FOVRange.Y) ResultDeltaFOV = FOVRange.Y - GetCameraComponent()->FieldOfView;
	if (GetCameraComponent()->FieldOfView + ResultDeltaFOV < FOVRange.X) ResultDeltaFOV = FOVRange.X - GetCameraComponent()->FieldOfView;
	return ResultDeltaFOV;
}

float UResolveGroupActorExtension::GetDistanceForDistanceOnly(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime, const float& FOV)
{
	/** Required distance. */
	float RequiredDistance = 114514.0f;
	for (FBoundingWrappedActor& BWActor : TargetActors)
	{
		FVector LocalSpacePosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), BWActor.Target->GetActorLocation());
		if (LocalSpacePosition.X > 0 && !BWActor.bExcludeBoundingBox)
		{
			float RightBound = (LocalSpacePosition.Y + BWActor.Width) * (1 - Tolerance);
			float LeftBound = (LocalSpacePosition.Y - BWActor.Width) * (1 - Tolerance);
			float TopBound = (LocalSpacePosition.Z + BWActor.Height) * (1 - Tolerance);
			float BottomBound = (LocalSpacePosition.Z - BWActor.Height) * (1 - Tolerance);
			
			float TanHalfFov = UKismetMathLibrary::DegTan(FOV / 2);
			if (RightBound > 0) RequiredDistance = FMath::Min(RequiredDistance, LocalSpacePosition.X - RightBound / TanHalfFov);
			if (LeftBound < 0) RequiredDistance = FMath::Min(RequiredDistance, LocalSpacePosition.X + LeftBound / TanHalfFov);
			if (TopBound > 0) RequiredDistance = FMath::Min(RequiredDistance, LocalSpacePosition.X - TopBound / TanHalfFov * GetCameraComponent()->AspectRatio);
			if (BottomBound < 0) RequiredDistance = FMath::Min(RequiredDistance, LocalSpacePosition.X + BottomBound / TanHalfFov * GetCameraComponent()->AspectRatio);
		}
	}
	/** Damp distance. */
	double ResultDeltaDistance;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, RequiredDistance, DistanceDampTime, ResultDeltaDistance);

	if (ResultDeltaDistance + AdjustedDistance > DistanceRange.Y) ResultDeltaDistance = DistanceRange.Y - AdjustedDistance;
	if (ResultDeltaDistance + AdjustedDistance < DistanceRange.X) ResultDeltaDistance = DistanceRange.X - AdjustedDistance;
	AdjustedDistance += ResultDeltaDistance;
	return ResultDeltaDistance;
}