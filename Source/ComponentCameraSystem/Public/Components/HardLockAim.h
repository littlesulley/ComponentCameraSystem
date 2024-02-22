// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentAim.h"
#include "Utils/ECameraLibrary.h"
#include "HardLockAim.generated.h"

/**
 * HardLockAim uses the rotation of aim target.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UHardLockAim : public UECameraComponentAim
{
	GENERATED_BODY()

public:
	UHardLockAim();

protected:
	/** Rotation offset applied to aim target's rotation. In aim target's local space. In the order of roll, pitch and yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HardLockAim")
	FRotator RotationOffset;

	/** Quaternion damping for camera to track the aim target's rotation. Small numbers indicate responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HardLockAim", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float QuatDamping;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
};