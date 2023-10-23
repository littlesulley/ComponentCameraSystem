// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ConstrainPitchExtension.h"

UConstrainPitchExtension::UConstrainPitchExtension()
{
	Stage = EStage::Noise;

	MinPitch = -80.0f;
	MaxPitch = 80.0f;
}

void UConstrainPitchExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (OwningActor->GetActorRotation().Pitch <= MaxPitch && OwningActor->GetActorRotation().Pitch >= MinPitch)
	{
		PrevRotation = OwningActor->GetActorRotation();
	}
	else
	{
		OwningActor->SetActorRotation(FRotator(PrevRotation.Pitch, OwningActor->GetActorRotation().Yaw, 0));
	}
}

void UConstrainPitchExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	PrevRotation = OwningActor->GetActorRotation();

	if (MinPitch > MaxPitch)
	{
		std::swap(MinPitch, MaxPitch);
	}
}