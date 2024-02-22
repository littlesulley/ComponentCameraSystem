// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EThirdPersonCamera.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "Components/ScreenFollow.h"
#include "Components/ControlAim.h"
#include "Extensions/ResolveOcclusionExtension.h"

AEThirdPersonCamera::AEThirdPersonCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ScreenFollowComponent     = CreateDefaultSubobject<UScreenFollow>("ScreenFollowComponent");
	auto ControlAimComponent       = CreateDefaultSubobject<UControlAim>("ControlAimComponent");
	auto ResolveOcclusionExtension = CreateDefaultSubobject<UResolveOcclusionExtension>("ResolveOcclusionExtension");

	CameraSettingsComponent->SetFollowComponent(ScreenFollowComponent);
	CameraSettingsComponent->SetAimComponent(ControlAimComponent);
	CameraSettingsComponent->AddExtension(ResolveOcclusionExtension);
}