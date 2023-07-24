// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/HardLockFollow.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/KismetMathLibrary.h"

UHardLockFollow::UHardLockFollow()
{
	Stage = EStage::Follow;
	FollowOffset = FVector(0.0f, 0.0f, 0.0f);
}

void UHardLockFollow::UpdateComponent_Implementation(float DeltaTime)
{
	if (FollowTarget != nullptr)
	{
		FVector FollowPosition = FollowTarget->GetActorLocation() + UKismetMathLibrary::GreaterGreater_VectorRotator(FollowOffset, FollowTarget->GetActorRotation());
		GetOwningActor()->SetActorLocation(FollowPosition);
	}
}