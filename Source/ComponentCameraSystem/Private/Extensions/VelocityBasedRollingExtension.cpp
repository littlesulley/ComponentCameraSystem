// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/VelocityBasedRollingExtension.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/ECameraLibrary.h"

UVelocityBasedRollingExtension::UVelocityBasedRollingExtension()
{
	// Should be applied at the last stage of the camera pipeline.
	Stage = EStage::Finalize;

	MaxRoll = 10.f;
	MinVelocity = 200.f;
	RollWaitTime = 0.5f;
	RestoreWaitTime = 0.2f;
	RollSceheme = ERollScheme::Constant;
	RollSpeed = 45.f;
	RollSpeedRatio = 0.01f;
	RollDamping = 1.f;
	RestoreSpeed = 100.f;
	RestoreDamping = 1.f;

	bRolling = false;
	ElapsedRollWaitTime = 0.f;
	ElapsedRestoreWaitTime = 0.f;
	CurrentRoll = 0.f;
	CachedFollowTargetLocation = FVector::ZeroVector;
}

void UVelocityBasedRollingExtension::UpdateComponent_Implementation(float DeltaTime)
{
	AActor* FollowTarget = nullptr;
	if (GetOwningSettingComponent()->GetFollowTarget() != nullptr)
	{
		FollowTarget = GetOwningSettingComponent()->GetFollowTarget();
	}

	if (FollowTarget)
	{
		FVector CurrentFollowTargetLocation = FollowTarget->GetActorLocation();
		FVector WorldVelocity = (CurrentFollowTargetLocation - CachedFollowTargetLocation) / DeltaTime;
		FVector LocalVelocity = UECameraLibrary::GetLocalSpacePositionWithVectors(
			FVector::ZeroVector,
			GetOwningActor()->GetActorForwardVector(),
			GetOwningActor()->GetActorRightVector(),
			GetOwningActor()->GetActorUpVector(),
			WorldVelocity
		);

		float CurrentVelocity = LocalVelocity.Y;

		if (FMath::Abs(CurrentVelocity) > MinVelocity)
		{
			// If in rolling, immediately add to roll.
			if (bRolling)
			{
				AddRoll(DeltaTime, CurrentVelocity);
			}
			// Otherwise, check elapsed wait time.
			else
			{
				ElapsedRollWaitTime += DeltaTime;

				// If pass, start rolling.
				if (ElapsedRollWaitTime >= RollWaitTime)
				{
					bRolling = true;
					AddRoll(DeltaTime, CurrentVelocity);
				}
			}
		}
		else if (bRolling)
		{
			// If roll is already close to zero, reset all cached variables.
			if (!ResetWhenRollIsSmall())
			{
				ElapsedRestoreWaitTime += DeltaTime;

				if (ElapsedRestoreWaitTime >= RestoreWaitTime)
				{
					RestoreRoll(DeltaTime);
				}
				else
				{
					RemainRoll();
				}
			}
		}

		CachedFollowTargetLocation = CurrentFollowTargetLocation;
	}
}

void UVelocityBasedRollingExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	bRolling = false;
	ElapsedRollWaitTime = 0.f;
	ElapsedRestoreWaitTime = 0.f;
	CurrentRoll = 0.f;
	CachedFollowTargetLocation = FVector::ZeroVector;
}

void UVelocityBasedRollingExtension::BindToOnPreTickComponent()
{
	FRotator CurrentRotation = GetOwningActor()->GetActorRotation();
	GetOwningActor()->SetActorRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, 0));
}

void UVelocityBasedRollingExtension::AddRoll(const float& DeltaTime, const float& CurrentVelocity)
{
	float RollAddition = 0.f;

	if (RollSceheme == ERollScheme::Constant)
	{
		RollAddition = FMath::Sign(CurrentVelocity) * RollSpeed * DeltaTime;
	}
	else if (RollSceheme == ERollScheme::ProportionalToVelocity)
	{
		RollAddition = RollSpeedRatio * CurrentVelocity;
	}

	float ExpectedRoll = FMath::Clamp(CurrentRoll + RollAddition, -MaxRoll, MaxRoll);

	float DeltaRoll = ExpectedRoll - CurrentRoll;
	float DampedDeltaRoll;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaRoll, RollDamping, DampedDeltaRoll);

	CurrentRoll += DampedDeltaRoll;
	GetOwningActor()->AddActorLocalRotation(FRotator(0, 0, CurrentRoll));

	// Reset ElapsedRestoreWaitTime.
	ElapsedRestoreWaitTime = 0.f;
}

void UVelocityBasedRollingExtension::RemainRoll()
{
	GetOwningActor()->AddActorLocalRotation(FRotator(0, 0, CurrentRoll));
}

void UVelocityBasedRollingExtension::RestoreRoll(const float& DeltaTime)
{
	float RollAddition = RestoreSpeed * DeltaTime;
	float ExpectedRoll = CurrentRoll > 0 ? FMath::Max<float>(0, CurrentRoll - RollAddition) : FMath::Min<float>(0, CurrentRoll + RollAddition);

	float DeltaRoll = ExpectedRoll - CurrentRoll;
	float DampedDeltaRoll;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaRoll, RestoreDamping, DampedDeltaRoll);

	CurrentRoll += DampedDeltaRoll;
	GetOwningActor()->AddActorLocalRotation(FRotator(0, 0, CurrentRoll));
}

bool UVelocityBasedRollingExtension::ResetWhenRollIsSmall()
{
	bool bSmallEnough = FMath::Abs(CurrentRoll) < 0.1 ? true : false;

	if (bSmallEnough)
	{
		bRolling = false;
		CurrentRoll = 0.f;
		ElapsedRollWaitTime = 0.f;
		ElapsedRestoreWaitTime = 0.f;
	}

	return bSmallEnough;
}