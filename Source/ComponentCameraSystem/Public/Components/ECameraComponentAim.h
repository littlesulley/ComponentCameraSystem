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

	virtual FVector GetRealAimPosition() 
	{ 
		if (IsSocketValid())
		{
			return GetSocketTransform().GetLocation();
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

		return true;
	}

	virtual FTransform GetSocketTransform()
	{
		UActorComponent* ActorComponent = AimTarget->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

		return SkeletonComponent->GetSocketTransform(SocketName);
	}
};
