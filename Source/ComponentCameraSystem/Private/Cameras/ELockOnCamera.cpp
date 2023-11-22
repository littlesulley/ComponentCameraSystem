// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/ELockOnCamera.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "Components/FramingFollow.h"
#include "Components/TargetingAim.h"
#include "Extensions/DeoccluderExtension.h"
#include "Extensions/ConstrainPitchExtension.h"
#include "Extensions/ModifyAimPointExtension.h"

AELockOnCamera::AELockOnCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ScreenFollowComponent     = CreateDefaultSubobject<UFramingFollow>("ScreenFollowComponent");
	auto TargetingAimComponent     = CreateDefaultSubobject<UTargetingAim>("TargetingAimComponent");
	auto ResolveOcclusionExtension = CreateDefaultSubobject<UDeoccluderExtension>("ResolveOcclusionExtension");
	auto ConstrainPitchExtension   = CreateDefaultSubobject<UConstrainPitchExtension>("ConstrainPitchExtension");
	auto ModifyAimPointExtension   = CreateDefaultSubobject<UModifyAimPointExtension>("ModifyAimPointExtension");

	CameraSettingsComponent->SetFollowComponent(ScreenFollowComponent);
	CameraSettingsComponent->SetAimComponent(TargetingAimComponent);
	CameraSettingsComponent->AddExtension(ResolveOcclusionExtension);
	CameraSettingsComponent->AddExtension(ConstrainPitchExtension);
	CameraSettingsComponent->AddExtension(ModifyAimPointExtension);
}