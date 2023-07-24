// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentFollow.h"
#include "HardLockFollow.generated.h"

/**
 * HardLockFollow uses the location of the follow target.
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
