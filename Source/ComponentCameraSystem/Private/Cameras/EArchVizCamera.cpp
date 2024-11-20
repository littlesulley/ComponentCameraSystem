// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EArchVizCamera.h"
#include "Cameras/EArchVizCameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"


AEArchVizCamera::AEArchVizCamera(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEArchVizCameraComponent>(TEXT("CameraComponent")))
{

}

void AEArchVizCamera::EndViewTarget(APlayerController* PC)
{
	FMinimalViewInfo& POV = PC->PlayerCameraManager->ViewTarget.POV;
	POV.OffCenterProjectionOffset = FVector2D(0, 0);
}