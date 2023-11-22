// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EThirdPersonCamera.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "Components/FramingFollow.h"
#include "Components/ControlAim.h"
#include "Extensions/DeoccluderExtension.h"

AEThirdPersonCamera::AEThirdPersonCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ScreenFollowComponent     = CreateDefaultSubobject<UFramingFollow>("ScreenFollowComponent");
	auto ControlAimComponent       = CreateDefaultSubobject<UControlAim>("ControlAimComponent");
	auto ResolveOcclusionExtension = CreateDefaultSubobject<UDeoccluderExtension>("ResolveOcclusionExtension");

	CameraSettingsComponent->SetFollowComponent(ScreenFollowComponent);
	CameraSettingsComponent->SetAimComponent(ControlAimComponent);
	CameraSettingsComponent->AddExtension(ResolveOcclusionExtension);
}