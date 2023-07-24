// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraComponentBase.h"
#include "Utils/ECameraTypes.h"
#include "ECameraExtensionBase.generated.h"

/**
 * Base class for all extensions.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraExtensionBase : public UECameraComponentBase
{
	GENERATED_BODY()

public:
	UECameraExtensionBase();
};
