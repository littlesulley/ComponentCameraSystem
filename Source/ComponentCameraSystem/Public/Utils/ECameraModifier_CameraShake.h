// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "ECameraModifier_CameraShake.generated.h"

class UCameraShakePattern; 

/**
 * Utility class to help simplify starting camera shakes.
 */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraModifier_CameraShake : public UCameraModifier_CameraShake
{
	GENERATED_BODY()
	
public:
	UECameraModifier_CameraShake(const FObjectInitializer& ObjectInitializer);

	/**
	 * Adds a new active screen shake to be applied using the Wave pattern. This version does not require creating a camera shake class in advance.
	 * @param InPattern - The input camera shake pattern.
	 * @param Params - The parameters for the new camera shake.
	 * @param bSingleInstance - If this camera shake should be singleton.
	 */
	virtual UCameraShakeBase* AddCameraShakeWave(UCameraShakePattern* InPattern, const FAddCameraShakeParams& Params, bool bSingleInstance = false);

	/**
	 * Adds a new active screen shake to be applied using the Perlin pattern. This version does not require creating a camera shake class in advance.
	 * @param InPattern - The input camera shake pattern.
	 * @param Params - The parameters for the new camera shake.
	 * @param bSingleInstance - If this camera shake should be singleton.
	 */
	virtual UCameraShakeBase* AddCameraShakePerlin(UCameraShakePattern* InPattern, const FAddCameraShakeParams& Params, bool bSingleInstance = false);
};
