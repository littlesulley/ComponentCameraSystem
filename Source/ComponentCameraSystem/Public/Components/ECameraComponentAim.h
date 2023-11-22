// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraComponentBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "ECameraComponentAim.generated.h"

/**
 * Base class for all aim components.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraComponentAim : public UECameraComponentBase
{
	GENERATED_BODY()

public:
	UECameraComponentAim();

protected:
	/** You MUST specify AimTarget in the midst of calling CallCamera node. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentAim")
	TWeakObjectPtr<AActor> AimTarget;

	/** Optional SocketName. 
	 *  If this socket can be found, its transform will be used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentAim")
	FName SocketName;

	/** Optional scene component.
	 *	If this is not null, its transform will be used.
	 *  Less prior than SocketName. If SocketName can be found, it will be used regardless of this component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraComponentAim")
	USceneComponent* SceneComponent;

public:
	virtual AActor* SetAimTarget(AActor* NewAimTarget)
	{
		AimTarget = NewAimTarget;
		return NewAimTarget;
	}

	virtual AActor* GetAimTarget()
	{
		return AimTarget.Get();
	}

	virtual FName SetAimSocket(FName NewAimSocket)
	{
		SocketName = NewAimSocket;
		return SocketName;
	}

	virtual FName GetAimSocket()
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

	virtual FVector GetRealAimPosition() 
	{ 
		if (IsSocketValid())
		{
			return GetSocketTransform().GetLocation();
		}
		else if (IsValid(SceneComponent))
		{
			return GetSceneComponentTransform().GetLocation();
		}
		else
		{
			return AimTarget != nullptr ? AimTarget->GetActorLocation() : GetOwningActor()->GetActorLocation();
		}
	}

	virtual FTransform GetRealAimTransform()
	{
		if (IsSocketValid())
		{
			return GetSocketTransform();
		}
		else if (IsValid(SceneComponent))
		{
			return GetSceneComponentTransform();
		}
		else
		{
			return AimTarget != nullptr ? AimTarget->GetActorTransform() : GetOwningActor()->GetActorTransform();
		}
	}

	virtual bool IsSocketValid()
	{
		if (SocketName.IsNone() || !AimTarget.IsValid())
		{
			return false;
		}

		UActorComponent* ActorComponent = AimTarget->GetComponentByClass(USkeletalMeshComponent::StaticClass());

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
		UActorComponent* ActorComponent = AimTarget->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

		return SkeletonComponent->GetSocketTransform(SocketName);
	}

	virtual FTransform GetSceneComponentTransform()
	{
		return SceneComponent->GetComponentTransform();
	}
};
