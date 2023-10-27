// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentFollow.h"
#include "HardLockFollow.generated.h"

/**
 * HardLockFollow uses the location of the follow target (or some socket).
 * It is the no-mask no-damping local-space version of SimpleFollow.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UHardLockFollow : public UECameraComponentFollow
{
	GENERATED_BODY()
	
public:
	UHardLockFollow();

protected:
	/** Camera offset in follow target's local space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HardLockFollow")
	FVector FollowOffset;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
};
