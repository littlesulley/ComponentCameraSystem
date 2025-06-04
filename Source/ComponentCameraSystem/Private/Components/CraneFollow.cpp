// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/CraneFollow.h"
#include "Utils/ECameraLibrary.h"
#include "CameraRig_Crane.h"

UCraneFollow::UCraneFollow()
{
	Stage = EStage::Follow;

	bLockOrientationOutwards = false;
	FollowType = ECraneFollowType::FollowTarget;
	bLockArmLength = false;
	bLockYaw = false;
	bLockPitch = false;
	StartPosition = FVector(100, 0, 0);
	Duration = FVector(0, 0, 0);
	Speed = FVector(0, 10, 0);
	bResetOnResume = true;

	bHasBegun = false;
	ElapsedTime = 0.0f;
	CurrentKeyPoint = -1;
	ElapsedBlendTime = 0;
	ElapsedDurationTime = 0;
}

void UCraneFollow::UpdateComponent_Implementation(float DeltaTime)
{
	/** If crane is null, return. */
	if (Crane == nullptr) return;

	/** If camera is currently blueprinted. */
	if (bIsBlueprinting)
	{
		ResolveWhenIsBlurprinting(DeltaTime);
		return;
	}

	if (FollowType == ECraneFollowType::FixedSpeed)
	{
		/** Move to start position. */
		if (!bHasBegun)
		{
			bHasBegun = true;
			Crane->CraneArmLength = StartPosition.X;
			Crane->CraneYaw = StartPosition.Y;
			Crane->CranePitch = StartPosition.Z;
		}

		/** Respectively for arm length, crane yaw and crane pitch. */
		for (int index = 0; index < 3; ++index)
		{
			if (ElapsedTime > Duration[index] && Duration[index] != 0.0f) continue;
			
			if (index == 0) Crane->CraneArmLength += Speed[0] * DeltaTime;
			if (index == 1) Crane->CraneYaw += Speed[1] * DeltaTime;
			if (index == 2) Crane->CranePitch += Speed[2] * DeltaTime;
		}

		/** Set new location and rotation. */
		UpdateAndSetTransform();
		ElapsedTime += DeltaTime;

		/** Update variables for HUD. */
		CraneLocation = Crane->GetActorLocation();
	}
	else if (FollowType == ECraneFollowType::FollowTarget)
	{
		if (FollowTarget == nullptr) return;

		/** Move to start position. */
		if (!bHasBegun)
		{
			bHasBegun = true;
			Crane->CraneArmLength = StartPosition.X;
			Crane->CraneYaw = StartPosition.Y;
			Crane->CranePitch = StartPosition.Z;
		}

		/** Apply follow offset, in world space. */
		FVector FollowPosition = FollowTarget->GetActorLocation() + FollowOffset;

		/** Set new crane position. */
		SetPositionToFollow(FollowPosition, DeltaTime);

		/** Set new location and rotation. */
		UpdateAndSetTransform();
	}
	else if (FollowType == ECraneFollowType::Manual)
	{
		/** Start position. */
		if (CurrentKeyPoint == -1)
		{
			Crane->CraneArmLength = ManualKeyPoints.StartPosition.X;
			Crane->CraneYaw = ManualKeyPoints.StartPosition.Y;
			Crane->CranePitch = ManualKeyPoints.StartPosition.Z;
			UpdateAndSetTransform();
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
			/** Has not finished duration yet. */
			else
			{
				/** Update elapsed blend time. */
				ElapsedBlendTime = FMath::Clamp(ElapsedBlendTime + DeltaTime, 0, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime);

				/** Get total move amount. */
				FVector Start = CurrentKeyPoint == 0 ? ManualKeyPoints.StartPosition : ManualKeyPoints.KeyPoints[CurrentKeyPoint - 1].Position;
				FVector End = ManualKeyPoints.KeyPoints[CurrentKeyPoint].Position;
				RenormalizeYawAndPitch(Start);
				RenormalizeYawAndPitch(End);
				FVector MoveAmount = GetMoveAmount(Start, End, ManualKeyPoints.KeyPoints[CurrentKeyPoint].bForwardYaw, ManualKeyPoints.KeyPoints[CurrentKeyPoint].bForwardPitch);
				
				/** Interpolation. */
				float DeltaArmLength = UKismetMathLibrary::Ease(0, MoveAmount[0], ElapsedBlendTime / ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendFunc);
				float DeltaYaw = UKismetMathLibrary::Ease(0, MoveAmount[1], ElapsedBlendTime / ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendFunc);
				float DeltaPitch = UKismetMathLibrary::Ease(0, MoveAmount[2], ElapsedBlendTime / ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendTime, ManualKeyPoints.KeyPoints[CurrentKeyPoint].BlendFunc);

				/** Apply deltas. */
				Crane->CraneArmLength = Start[0] + DeltaArmLength;
				Crane->CraneYaw = Start[1] + DeltaYaw;
				Crane->CranePitch = Start[2] + DeltaPitch;
				UpdateAndSetTransform();
			}
		}
	}
}

FTransform UCraneFollow::UpdateTransformOnCrane()
{
	FRotator NewYawControlRot = FRotator(0.f, Crane->CraneYaw, 0.f);
	FRotator NewPitchControlRot = FRotator(Crane->CranePitch, 0.f, 0.f);

	FQuat NewQuat = NewYawControlRot.Quaternion() * NewPitchControlRot.Quaternion();
	FVector LocationOffset = UKismetMathLibrary::GetForwardVector(NewQuat.Rotator()) * Crane->CraneArmLength;
	FVector NewLocation = LocationOffset + Crane->GetActorLocation();

	return FTransform(NewQuat, NewLocation);
}

void UCraneFollow::UpdateAndSetTransform()
{
	FTransform NewTransform = UpdateTransformOnCrane();
	GetOwningActor()->SetActorLocation(NewTransform.GetLocation());
	if (bLockOrientationOutwards)
		GetOwningActor()->SetActorRotation(NewTransform.GetRotation());
}

void UCraneFollow::SetPositionToFollow(const FVector& FollowPosition, float DeltaTime)
{
	/** Get desired arm length, crane yaw and crane pitch. */
	float DesiredArmLength = UKismetMathLibrary::Vector_Distance(Crane->GetActorLocation(), FollowPosition);
	FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(Crane->GetActorLocation(), FollowPosition);
	float DesiredYaw = DesiredRotation.Yaw;
	float DesiredPitch = DesiredRotation.Pitch;

	/** Apply damped delta. */
	if (!bLockArmLength) Crane->CraneArmLength = DesiredArmLength;
	if (!bLockYaw) Crane->CraneYaw = DesiredYaw;
	if (!bLockPitch) Crane->CranePitch = DesiredPitch;
}

void UCraneFollow::RenormalizeYawAndPitch(FVector& Input)
{
	while (Input[1] < -180) Input[1] += 360;
	while (Input[1] > 180) Input[1] -= 360;
	while (Input[2] < -180) Input[2] += 360;
	while (Input[2] > 180) Input[2] -= 360;
}

FVector UCraneFollow::GetMoveAmount(const FVector& Start, const FVector& End, bool bForwardYaw, bool bForwardPitch)
{
	FVector MoveAmount = FVector();

	MoveAmount[0] = End[0] - Start[0];
	if (bForwardYaw) MoveAmount[1] = End[1] >= Start[1] ? End[1] - Start[1] : 180 - Start[1] + End[1] + 180;
	else MoveAmount[1] = End[1] <= Start[1] ? End[1] - Start[1] : -180 - Start[1] + End[1] - 180;
	if (bForwardPitch) MoveAmount[2] = End[2] >= Start[2] ? End[2] - Start[2] : 180 - Start[2] + End[2] + 180;
	else MoveAmount[2] = End[2] <= Start[2] ? End[2] - Start[2] : -180 - Start[2] + End[2] - 180;

	return MoveAmount;
}

void UCraneFollow::ResolveWhenIsBlurprinting(float DeltaTime)
{

}