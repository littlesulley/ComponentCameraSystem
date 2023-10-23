// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "ConstrainPitchExtension.generated.h"

/**
 * This extension is used to constrain pitch within a given range. 
 * Damping should be applied to ensure a smooth transition.
 * Ideally, this extension should only be used for lock-on cameras.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UConstrainPitchExtension : public UECameraExtensionBase
{
	GENERATED_BODY()

public:
	UConstrainPitchExtension();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ConstrainPitch", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float MinPitch = -80;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ConstrainPitch", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float MaxPitch = 80;

private:
	FRotator PrevRotation;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

};
