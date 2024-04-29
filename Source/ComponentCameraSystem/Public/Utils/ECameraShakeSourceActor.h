// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeSourceActor.h"
#include "ECameraShakeSourceActor.generated.h"

/**
 * This is a specialized actor class used for function EasyStartCameraShakeFromSource.
 */
UCLASS(NotBlueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AECameraShakeSourceActor : public ACameraShakeSourceActor
{
	GENERATED_BODY()

};
