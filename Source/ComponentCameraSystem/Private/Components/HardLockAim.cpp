// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/HardLockAim.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


UHardLockAim::UHardLockAim()
{
	Stage = EStage::Aim;
	RotationOffset = FRotator(0, 0, 0);
	QuatDamping = 1.0f;
}

void UHardLockAim::UpdateComponent_Implementation(float DeltaTime)
{
	if (AimTarget != nullptr)
	{
		/** Get desired rotation and quaternion. */
		FQuat CurrentQuat = GetOwningActor()->GetActorQuat();
		FQuat DesiredQuat = GetRealAimTransform().GetRotation() * FQuat(RotationOffset);

		FQuat DampedQuat;
		UECameraLibrary::DamperQuaternion(CurrentQuat, DesiredQuat, DeltaTime, QuatDamping, DampedQuat);
		GetOwningActor()->SetActorRotation(DampedQuat);
	}
}