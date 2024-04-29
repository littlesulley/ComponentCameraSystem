// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraBase.h"
#include "ELockOnCamera.generated.h"

/**
 * This class is specially used for lock-on purposes, in cases when fighting with a BOSS.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AELockOnCamera : public AECameraBase
{
	GENERATED_BODY()

public:
	AELockOnCamera(const FObjectInitializer& ObjectInitializer);
};
