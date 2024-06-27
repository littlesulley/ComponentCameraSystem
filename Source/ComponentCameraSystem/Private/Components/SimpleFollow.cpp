// Copyright 2023 by Sulley. All Rights Reserved.


#include "Components/SimpleFollow.h"
#include "Utils/ECameraTypes.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"

USimpleFollow::USimpleFollow()
{
	Stage = EStage::Follow;
	FollowType = ESimpleFollowType::WorldSpace;
	AxisMasks = FVector(1.0f, 1.0f, 1.0f);
	Damper = CreateDefaultSubobject<UECameraDamperVector>("Camera Damper Vector");
}

void USimpleFollow::UpdateComponent_Implementation(float DeltaTime)
{
	if (FollowTarget != nullptr)
	{
		/** Get the *real* follow position, depending on FollowType. */
		FVector FollowPosition = GetRealFollowPosition(true);

		/** Transform from world space to local space. */
		FVector LocalSpaceFollowPosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), FollowPosition);

		/** Temporary (before damping) delta position. */
		FVector TempDeltaPosition = LocalSpaceFollowPosition;

		/** Get damped delta position. */
		FVector DampedDeltaPosition = DampDeltaPosition(TempDeltaPosition, DeltaTime);

		/** Transform from local space back to world space. */
		FVector WorldDeltaPosition = UKismetMathLibrary::TransformDirection(GetOwningActor()->GetActorTransform(), DampedDeltaPosition);

		/** Apply axis masks. */
		FVector MaskedDeltaPosition = ApplyAxisMask(WorldDeltaPosition);

		/** Apply damped delta position. */
		GetOwningActor()->AddActorWorldOffset(MaskedDeltaPosition);
	}
}

FVector USimpleFollow::GetRealFollowPosition(bool bWithOffset)
{
	FVector Position = FVector();
	FRotator Rotation = FRotator();

	if (IsSocketValid())
	{
		FTransform SocketTransform = GetSocketTransform();
		Position = SocketTransform.GetLocation();
		Rotation = SocketTransform.Rotator();
	}
	else
	{
		if (IsValid(SceneComponent))
		{
			FTransform ComponentTransform = GetSceneComponentTransform();
			Position = ComponentTransform.GetLocation();
			Rotation = ComponentTransform.Rotator();
		}
		else if (FollowTarget != nullptr)
		{
			Position = GetFollowTarget()->GetActorLocation();
			Rotation = GetFollowTarget()->GetActorRotation();
		}
		else
		{
			Position = GetOwningActor()->GetActorLocation();
			Rotation = GetOwningActor()->GetActorRotation();
		}
	}

	if (FollowType == ESimpleFollowType::WorldSpace)
	{
		if (bWithOffset)
		{
			return Position + FollowOffset;
		}
		else
		{
			return Position;
		}
	}
	else
	{
		if (bWithOffset)
		{
			return UECameraLibrary::GetPositionWithLocalRotatedOffset(Position, Rotation, FollowOffset);
		}
		else
		{
			return Position;
		}
	}
}

FVector USimpleFollow::DampDeltaPosition(const FVector& TempDeltaPosition, float DeltaTime)
{
	FVector DampedDeltaPosition = TempDeltaPosition;

	if (IsValid(Damper))
	{
		Damper->SetInput(TempDeltaPosition);
		DampedDeltaPosition = Damper->ApplyDamp(DeltaTime);
		Damper->SetOutput(DampedDeltaPosition);
		Damper->PostApplyDamp();
	}

	return DampedDeltaPosition;
}

FVector USimpleFollow::ApplyAxisMask(const FVector& DampedDeltaPosition)
{
	if (FollowType == ESimpleFollowType::WorldSpace)
	{
		FVector BinaryAxisMasks = FVector(AxisMasks[0] != 0.0f, AxisMasks[1] != 0.0f, AxisMasks[2] != 0.0f);
		return BinaryAxisMasks * DampedDeltaPosition + BinaryAxisMasks * FollowOffset;
	}
	/** AxisMasks is only applied to WorldSpace. */
	else return DampedDeltaPosition;
}