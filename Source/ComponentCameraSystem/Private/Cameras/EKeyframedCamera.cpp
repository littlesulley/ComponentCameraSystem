// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EKeyframedCamera.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "ActorSequenceComponent.h"
#include "Extensions/KeyframeExtension.h"

AEKeyframedCamera::AEKeyframedCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto KeyframeExtension = CreateDefaultSubobject<UKeyframeExtension>("KeyframeExtension");
	CameraSettingsComponent->AddExtension(KeyframeExtension);
}