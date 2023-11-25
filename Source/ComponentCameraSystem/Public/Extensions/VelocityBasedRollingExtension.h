// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "VelocityBasedRollingExtension.generated.h"

/**
 * This extension is used to add roll to camera based on the FOLLOW TARGET's velocity.
 * You should make sure the follow target is valid.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UVelocityBasedRollingExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UVelocityBasedRollingExtension();

protected:


public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;
};
