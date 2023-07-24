// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "EArchVizCineCamera.generated.h"

/**
 * This is a specialized camera used for an architectural visulization view in cinematics.
 * You should key-frame the `PerspectiveStrength` value to adjust the strength for two-point perspective.
 * If you want to customize this class, inherit and modify EArchVizCineCameraComponent.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEArchVizCineCamera : public ACineCameraActor
{
	GENERATED_BODY()
	
public:
	AEArchVizCineCamera(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "ECamera|ArchViz")
	class UEArchVizCineCameraComponent* GetArchVizCameraComponent() { return Cast<UEArchVizCineCameraComponent>(Super::GetCameraComponent()); }

	virtual void EndViewTarget(class APlayerController* PC) override;
};
