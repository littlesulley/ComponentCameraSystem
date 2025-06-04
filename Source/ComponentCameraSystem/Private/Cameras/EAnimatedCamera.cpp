// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EAnimatedCamera.h"
#include "Core/ECameraBase.h"
#include "Extensions/AnimatedCameraExtension.h"

AEAnimatedCamera::AEAnimatedCamera(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	auto AnimatedCameraExtension = CreateDefaultSubobject<UAnimatedCameraExtension>(TEXT("AnimatedCameraExtension"));
	CameraSettingsComponent->AddExtension(AnimatedCameraExtension);
}