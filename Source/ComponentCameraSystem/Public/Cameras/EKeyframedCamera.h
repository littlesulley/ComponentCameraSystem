// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraBase.h"
#include "EKeyframedCamera.generated.h"

/**
 * This class is specially used for keyframed cameras.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEKeyframedCamera : public AECameraBase
{
	GENERATED_BODY()

public:
	AEKeyframedCamera(const FObjectInitializer& ObjectInitializer);
};
