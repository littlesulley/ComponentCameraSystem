// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraComponentBase.h"
#include "Core/ECameraSettingsComponent.h"
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
};
