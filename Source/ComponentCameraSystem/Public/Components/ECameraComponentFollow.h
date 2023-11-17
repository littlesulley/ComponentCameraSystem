// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraComponentBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Utils/ECameraLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "ECameraComponentFollow.generated.h"

/**
 * Base class for all follow components.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraComponentFollow : public UECameraComponentBase
{
	GENERATED_BODY()

public:
	UECameraComponentFollow();

protected:
	/** You MUST specify FollowTarget in the midst of calling CallCamera node. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentFollow")
	TWeakObjectPtr<AActor> FollowTarget;

	/** Optional SocketName. 
	 *  If this socket can be found, its transform will be used.
	 *  You should be very CAREFUL of the socket's rotation, since the final location is based on the local space.
	 *  DOES NOT apply to OrbitFollow and CraneFollow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentFollow")
	FName SocketName;

public:
	virtual AActor* SetFollowTarget(AActor* NewFollowTarget)
	{
		FollowTarget = NewFollowTarget;
		return NewFollowTarget;
	}

	virtual AActor* GetFollowTarget()
	{
		return FollowTarget.Get();
	}

	virtual FName SetFollowSocket(FName NewFollowSocket)
	{
		SocketName = NewFollowSocket;
		return SocketName;
	}

	virtual FName GetFollowSocket()
	{
		return SocketName;
	}

	virtual FVector GetRealFollowPosition(const FVector& Offset)
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
			if (FollowTarget != nullptr)
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

		return UECameraLibrary::GetPositionWithLocalRotatedOffset(Position, Rotation, Offset);
	}

	virtual bool IsSocketValid()
	{
		if (SocketName.IsNone() || !FollowTarget.IsValid())
		{
			return false;
		}

		UActorComponent* ActorComponent = FollowTarget->GetComponentByClass(USkeletalMeshComponent::StaticClass());

		if (ActorComponent == nullptr)
		{
			return false;
		}

		USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

		if (SkeletonComponent->DoesSocketExist(SocketName))
		{
			return true;
		}

		return false;
	}

	virtual FTransform GetSocketTransform()
	{
		UActorComponent* ActorComponent = FollowTarget->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

		return SkeletonComponent->GetSocketTransform(SocketName);
	}
};
