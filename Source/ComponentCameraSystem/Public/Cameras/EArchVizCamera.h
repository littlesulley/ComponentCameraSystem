// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ECameraBase.h"
#include "EArchVizCamera.generated.h"

/**
 * This is a specialized camera used for an architectural visulization view. 
 * You should tweak the `PerspectiveStrength` value to adjust the strength for two-point perspective.
 * If you want to customize this class, inherit and modify EArchVizCameraComponent.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEArchVizCamera : public AECameraBase
{
	GENERATED_BODY()

public:
	AEArchVizCamera(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "ECamera|ArchViz")
	class UEArchVizCameraComponent* GetArchVizCameraComponent() { return Cast<UEArchVizCameraComponent>(Super::GetCameraComponent()); }

	virtual void EndViewTarget(APlayerController* PC) override;
};
