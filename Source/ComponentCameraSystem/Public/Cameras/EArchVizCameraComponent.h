// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CineCameraComponent.h"
#include "EArchVizCameraComponent.generated.h"

/**
 * Camera component actually included in EArchVizCamera actors.
 * You should tweak the `PerspectiveStrength` value to adjust the strength for two-point perspective.
 * You can inherit and modify this class to add extra logic.
 */
UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera", meta = (BlueprintSpawnableComponent))
class COMPONENTCAMERASYSTEM_API UEArchVizCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UEArchVizCameraComponent(const FObjectInitializer& ObjectInitializer);

public:
	/** Whether to enable perspective correction. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category = "ArchVizCameraComponent")
	bool bEnableCorrection;

	/** Strength to correct perspective. 1.0 means full correction. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category = "ArchVizCameraComponent", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableCorrection"))
	float PerspectiveStrength;

	/** Vertical project offset. Used for debug view only. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Interp, Category = "ArchVizCameraComponent")
	float VerticalProjectOffset;

	/** Max and min pitch, should be propertly set to avoid distortion. 
	  * If also using ControlAim, this parameter should be compatible with that in ControlAim.
	  */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category = "ArchVizCameraComponent")
	FVector2D MinMaxPitch;

public:
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
