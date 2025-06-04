// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraBase.h"
#include "EThirdPersonCamera.generated.h"

/**
 * This class is specially used for third person cameras, with components ScreenFollow and ControlAim.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEThirdPersonCamera : public AECameraBase
{
	GENERATED_BODY()
	
public:
	AEThirdPersonCamera(const FObjectInitializer& ObjectInitializer);
};
