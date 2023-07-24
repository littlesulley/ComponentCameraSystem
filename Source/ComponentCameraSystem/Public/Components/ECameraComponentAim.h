// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraComponentBase.h"
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

public:
	virtual FVector GetRealAimPosition() 
	{ 
		if (AimTarget != nullptr)
			return AimTarget->GetActorLocation();
		else return GetOwningActor()->GetActorLocation();
	}
};
