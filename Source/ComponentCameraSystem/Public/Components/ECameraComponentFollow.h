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

	/** Follow target's local space offset applied to the follow target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraComponentFollow")
	FVector FollowOffset {0.0, 0.0, 0.0};

	/** Optional SocketName. 
	 *  If this socket can be found, its transform will be used.
	 *  You should be very CAREFUL of the socket's rotation, since the final location is based on the local space.
	 *  DOES NOT apply to OrbitFollow and CraneFollow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentFollow")
	FName SocketName;

	/** Optional scene component.
	 *	If this is not null, its transform will be used.
	 *  The same as SocketName, you should be very CAREFUL of its rotation, since the final location is based on the local space.
	 *  DOES NOT apply to OrbitFollow and CraneFollow.
	 *  Less prior than SocketName. If SocketName can be found, it will be used regardless of this component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentFollow")
	TObjectPtr<USceneComponent> SceneComponent;

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

	FVector SetFollowOffset(FVector InFollowOffset)
	{
		FollowOffset = InFollowOffset;
		return FollowOffset;
	}

	FVector GetFollowOffset()
	{
		return FollowOffset;
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

	virtual USceneComponent* SetSceneComponent(USceneComponent* NewSceneComponent)
	{
		SceneComponent = NewSceneComponent;
		return SceneComponent;
	}

	virtual USceneComponent* GetSceneComponent()
	{
		return SceneComponent;
	}

	virtual FVector GetRealFollowPosition(bool bWithOffset)
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

		if (bWithOffset)
		{
			return UECameraLibrary::GetPositionWithLocalRotatedOffset(Position, Rotation, FollowOffset);
		}
		else
		{
			return Position;
		}
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


	virtual FTransform GetSceneComponentTransform()
	{
		return SceneComponent->GetComponentTransform();
	}
};
