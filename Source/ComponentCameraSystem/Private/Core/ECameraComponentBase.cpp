// Copyright 2023 by Sulley. All Rights Reserved.


#include "Core/ECameraComponentBase.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UECameraComponentBase::UECameraComponentBase() { }

bool UECameraComponentBase::Valid()
{
	return true;
}

void UECameraComponentBase::BeginDestroy()
{
	Super::BeginDestroy();
}

void UECameraComponentBase::BindToOnPreTickComponent()
{
	K2_BindToOnPreTickComponent();
}

void UECameraComponentBase::BindToOnPostTickComponent()
{
	K2_BindToOnPostTickComponent();
}

void UECameraComponentBase::BindToOnBecomeViewTarget(APlayerController* PC)
{
	K2_BindToOnBecomeViewTarget(PC);
}

void UECameraComponentBase::BindToOnEndViewTarget(APlayerController* PC)
{
	K2_BindToOnEndViewTarget(PC);
}