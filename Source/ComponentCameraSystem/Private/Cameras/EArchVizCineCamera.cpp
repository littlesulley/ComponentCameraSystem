// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EArchVizCineCamera.h"
#include "Cameras/EArchVizCineCameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"

AEArchVizCineCamera::AEArchVizCineCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEArchVizCineCameraComponent>(TEXT("CameraComponent")))
{

}

void AEArchVizCineCamera::EndViewTarget(APlayerController* PC)
{
	FMinimalViewInfo& POV = PC->PlayerCameraManager->ViewTarget.POV;
	POV.OffCenterProjectionOffset = FVector2D(0, 0);
}