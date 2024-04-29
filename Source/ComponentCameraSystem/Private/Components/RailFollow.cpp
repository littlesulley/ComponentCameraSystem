// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/RailFollow.h"
#include "Utils/ECameraLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "CameraRig_Rail.h"

URailFollow::URailFollow()
{
	Stage = EStage::Follow;

	bLockOrientationToRail = false;
	FollowType = ERailFollowType::FollowTarget;
	Damping = 0.5;
	MaxVelocity = 1000.0;
	Duration = 0.0;
	StartPosition = 0.0;
	Speed = 0.1;
	bLoop = true;
	bResetOnResume = true;

	bIsBlueprinting = false;
	PreviousPositionOnRail = -1;
	CurrentPositionOnRail = -1;
	ElapsedTime = 0.0;
	ElapsedBlendTime = 0.0f;
	ElapsedDurationTime = 0.0f;
	CurrentKeyPoint = -1;
}

void URailFollow::UpdateComponent_Implementation(float DeltaTime)
{
	/** If rail is not valid, return. */
	if (!Rail.IsValid()) return;

	/** If camera is currently blueprinted. */
	if (bIsBlueprinting)
	{
		ResolveWhenIsBlueprinting(DeltaTime);
		return;
	}

	/** FollowType is FixedSpeed. */
	if (FollowType == ERailFollowType::FixedSpeed)
	{
		if (Duration != 0.0 && ElapsedTime > Duration) return;

		/** Get current position on rail. */
		if (PreviousPositionOnRail == -1) PreviousPositionOnRail = StartPosition;
		CurrentPositionOnRail = PreviousPositionOnRail + DeltaTime * Speed;

		/** Rectify current position according to Loop. */
		if (bLoop)
		{
			if (CurrentPositionOnRail > 1.0f) CurrentPositionOnRail -= 1.0f;
			if (CurrentPositionOnRail < 0.0f) CurrentPositionOnRail += 1.0f;
		}
		else
		{
			if (CurrentPositionOnRail > 1.0f) CurrentPositionOnRail = 1.0f;
			if (CurrentPositionOnRail < 0.0f) CurrentPositionOnRail = 0.0f;
		}

		/** Set current and previous position on rail. */
		PreviousPositionOnRail = CurrentPositionOnRail;
		SetRailPositionAndUpdateCameraTransform(CurrentPositionOnRail);

		ElapsedTime += DeltaTime;
	}
	/** FollowType is FollowTarget. */
	else if (FollowType == ERailFollowType::FollowTarget)
	{
		if (FollowTarget != nullptr)
		{
			float TargetPosition = GetNormalizedPositionOnRailNearestToTarget(FollowTarget.Get());
			
			float DeltaPosition = TargetPosition - Rail->CurrentPositionOnRail;
			double DampedDeltaPosition;
			UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaPosition, Damping, DampedDeltaPosition);

			/** Apply post restrictions. */
			ApplyPostRestrictions(DeltaTime, DampedDeltaPosition);

			/** Update location and rotation. */
			Rail->CurrentPositionOnRail += DampedDeltaPosition;

			FTransform DesiredTransform = UpdateTransformOnRail();
			GetOwningActor()->SetActorLocation(DesiredTransform.GetLocation());

			/** Check bLockOrientationToRail. */
			if (bLockOrientationToRail)
			{
				GetOwningActor()->SetActorRotation(DesiredTransform.GetRotation());
			}
		}
	}
	/** FollowType is Manual. */
	else if (FollowType == ERailFollowType::Manual)
	{
		/** Start point. */
		if (CurrentKeyPoint == -1)
		{
			SetRailPositionAndUpdateCameraTransform(ManualKeyPoints.StartPosition);
			if (ManualKeyPoints.KeyPoints.Num() > 0) CurrentKeyPoint = 0;
		}
		/** Move camera according to key points. */
		else 
		{
			/** Exceed max number of key points, return. */
			int NumberOfKeyPoints = ManualKeyPoints.KeyPoints.Num();
			if (CurrentKeyPoint >= NumberOfKeyPoints) return;

			/** Has finished blending. */
			if (ElapsedBlendTime >= ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime)
			{
				/** Has finished duration, move to next key point. */
				if (ElapsedDurationTime >= ManualKeyPoints.KeyPoints[CurrentKeyPoint].Duration)
				{
					++CurrentKeyPoint;
					ElapsedBlendTime = 0;
					ElapsedDurationTime = 0;
					return;
				}
				/** Has not finished duration yet, do nothing but wait. */
				else
				{
					ElapsedDurationTime += DeltaTime;
					return;
				}
			}
			/** Has not finished blending yet. */
			else
			{
				/** Update elapsed blend time. */
				ElapsedBlendTime = FMath::Clamp(ElapsedBlendTime + DeltaTime, 0, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime);
				
				/** Get total move amount. */
				float Start = CurrentKeyPoint == 0 ? ManualKeyPoints.StartPosition : ManualKeyPoints.KeyPoints[CurrentKeyPoint - 1].Position;
				float End = ManualKeyPoints.KeyPoints[CurrentKeyPoint].Position;
				float MoveAmount = GetMoveAmount(Start, End, ManualKeyPoints.KeyPoints[CurrentKeyPoint].bForward);
				
				/** Interpolation. */
				float DeltaPosition = UKismetMathLibrary::Ease(0, MoveAmount, ElapsedBlendTime / ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendFunc);
				
				/** Apply delta position. */
				float ResultPosition = Start + DeltaPosition;
				if (ResultPosition > 1.0f) ResultPosition -= 1.0f;
				if (ResultPosition < 0.0f) ResultPosition += 1.0f;
				SetRailPositionAndUpdateCameraTransform(ResultPosition);
			}
		}
	}
}

FTransform URailFollow::UpdateTransformOnRail()
{
	if (!GetWorld())
	{
		return FTransform();
	}

	if (Rail)
	{
		USplineComponent* Spline = Rail->GetRailSplineComponent();

		float const SplineLen = Spline->GetSplineLength();
		FVector const CameraPos = Spline->GetLocationAtDistanceAlongSpline(Rail->CurrentPositionOnRail * SplineLen, ESplineCoordinateSpace::World);

		if (bLockOrientationToRail)
		{
			FQuat const CameraRot = Spline->GetQuaternionAtDistanceAlongSpline(Rail->CurrentPositionOnRail * SplineLen, ESplineCoordinateSpace::World);
			return FTransform(CameraRot, CameraPos);
		}
		else
		{
			return FTransform(FQuat(), CameraPos);
		}
	}
	else return FTransform();
}

float URailFollow::GetNormalizedPositionOnRailNearestToTarget(AActor* Target)
{
	/** Get the underlying spline. */
	USplineComponent* Spline = Rail->GetRailSplineComponent();

	/** Get posiiton on rail. */
	FVector FollowPosition = FVector::ZeroVector;

	if (Target == FollowTarget.Get())
	{ 
		FollowPosition = GetRealFollowPosition(false);
	}
	else
	{
		FollowPosition = Target->GetActorLocation();
	}

	float InputKey = Spline->FindInputKeyClosestToWorldLocation(FollowPosition);
	float Distance = Spline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
	float Position = Distance / Spline->GetSplineLength();

	return Position;
}

void URailFollow::SetRailPositionAndUpdateCameraTransform(float Position)
{
	Rail->CurrentPositionOnRail = Position;

	/** Set camera location (and rotation) at start point. */
	FTransform CurrentTransform = UpdateTransformOnRail();
	GetOwningActor()->SetActorLocation(CurrentTransform.GetLocation());
	if (bLockOrientationToRail)
		GetOwningActor()->SetActorRotation(CurrentTransform.GetRotation());
}

FVector URailFollow::GetDampedDeltaLocationInWorldSpace(float DeltaTime, float DampingTime, FTransform& DesiredTransform)
{
	DesiredTransform = UpdateTransformOnRail();
	FVector DesiredLocation = DesiredTransform.GetLocation();
	FVector RawDeltaLocation = DesiredLocation - GetOwningActor()->GetActorLocation();

	FVector DampedDeltaLocation;
	UECameraLibrary::DamperVectorWithSameDampTime(FDampParams(), DeltaTime, RawDeltaLocation, DampingTime, DampedDeltaLocation);

	return DampedDeltaLocation;
}

float URailFollow::GetDampedDeltaPositionOnRail(float Start, float End, bool bIsForward, float DeltaTime, float DampingTime)
{
	float DeltaPosition = GetMoveAmount(Start, End, bIsForward);
	double DampedDeltaPosition;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaPosition, DampingTime, DampedDeltaPosition);
	return DampedDeltaPosition;
}

float URailFollow::GetMoveAmount(float Start, float End, bool bIsForward)
{
	float Amount = 0.0f;
	if (bIsForward) Amount = End >= Start ? End - Start : 1.0f - Start + End;
	else Amount = End <= Start ? End - Start : 0.0f - Start + End - 1.0f;
	return Amount;
}

void URailFollow::ApplyPostRestrictions(const float& DeltaTime, double& DeltaPosition)
{
	USplineComponent* Spline = Rail->GetRailSplineComponent();
	float SplineLength = Spline->GetSplineLength();

	float CurrentPosition = Rail->CurrentPositionOnRail;
	float CurrentDistance = CurrentPosition * SplineLength;
	float ExpectedPosition = CurrentPosition + DeltaPosition;
	float ExpectedDistance = ExpectedPosition * SplineLength;
	
	float Velocity = (ExpectedDistance - CurrentDistance) / DeltaTime;

	if (FMath::Abs(Velocity) > MaxVelocity)
	{
		if (ExpectedPosition > CurrentPosition)
		{
			ExpectedDistance = CurrentDistance + DeltaTime * MaxVelocity;
		}
		else
		{
			ExpectedDistance = CurrentDistance - DeltaTime * MaxVelocity;
		}

		ExpectedPosition = ExpectedDistance / SplineLength;
		DeltaPosition = ExpectedPosition - CurrentPosition;
	}
}

void URailFollow::ResolveWhenIsBlueprinting(float DeltaTime)
{
	ElapsedTimeWhenIsBlueprinting = FMath::Clamp(DeltaTime + ElapsedTimeWhenIsBlueprinting, 0, BlendTimeWhenIsBlueprinting);

	float MoveAmount = GetMoveAmount(StartPositionWhenIsBlueprinting, TargetPositionWhenIsBlueprinting, bIsForwardWhenIsBlueprinting);
	float DeltaPosition = UKismetMathLibrary::Ease(0, MoveAmount, ElapsedTimeWhenIsBlueprinting / BlendTimeWhenIsBlueprinting, BlendFuncWhenIsBlueprinting);
	float ResultPosition = StartPositionWhenIsBlueprinting + DeltaPosition;
	if (ResultPosition > 1.0f) ResultPosition -= 1.0f;
	if (ResultPosition < 0.0f) ResultPosition += 1.0f;
	SetRailPositionAndUpdateCameraTransform(ResultPosition);
}

void URailFollow::SetPositionOnRailWithDamping(const UObject* WorldContextObject, float Position, float BlendTime, TEnumAsByte<EEasingFunc::Type> BlendFunc, bool bIsForward)
{
	bIsBlueprinting = true;
	TargetPositionWhenIsBlueprinting = Position;
	BlendTimeWhenIsBlueprinting = BlendTime;
	BlendFuncWhenIsBlueprinting = BlendFunc;
	bIsForwardWhenIsBlueprinting = bIsForward;
	ElapsedTimeWhenIsBlueprinting = UGameplayStatics::GetWorldDeltaSeconds(WorldContextObject);

	/** Get rail position nearest to camera in case camera is currently not precisely on rail. */
	StartPositionWhenIsBlueprinting = Rail->CurrentPositionOnRail; 

	/** Get and apply damped delta position. */
	float MoveAmount = GetMoveAmount(StartPositionWhenIsBlueprinting, Position, bIsForward);
	float DeltaPosition = UKismetMathLibrary::Ease(0, MoveAmount, ElapsedTimeWhenIsBlueprinting / BlendTime, BlendFunc);
	float ResultPosition = StartPositionWhenIsBlueprinting + DeltaPosition;
	if (ResultPosition > 1.0f) ResultPosition -= 1.0f;
	if (ResultPosition < 0.0f) ResultPosition += 1.0f;
	SetRailPositionAndUpdateCameraTransform(ResultPosition);
}

void URailFollow::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	if (bResetOnResume)
	{
		bIsBlueprinting = false;
		PreviousPositionOnRail = -1;
		CurrentPositionOnRail = -1;
		ElapsedTime = 0.0;
		ElapsedBlendTime = 0.0f;
		ElapsedDurationTime = 0.0f;
		CurrentKeyPoint = -1;

		if (FollowTarget.IsValid() && Rail.IsValid())
		{
			Rail->CurrentPositionOnRail = GetNormalizedPositionOnRailNearestToTarget(FollowTarget.Get());
		}
	}
}