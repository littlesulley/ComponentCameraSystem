// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "ModifyAimPointExtension.generated.h"

class UFramingFollow;
class UTargetingAim;

/**
 * This extension can also be used to constrain pitch, but in a different way:
 *     When the follow position is within a given radius of the aim position,
 *	   the aim position will be pushed away from the camera, ensuring camera jitters won't happen.
 *	   This is achieved by interpolating between two types of additional aim offsets, which you can see 
 *	   from the read-only property AdditionalAimOffset in the TargetingAim component.
 *     1) The first type is FixedAddition, which tries to keep a fixed angle between the follow position
 *     and the aim position after applied with this kind of additional aim offset.
 *     2) The second type is DynamicAddition, which tries to keep a fixed aim position in screen space,
 *	   because FixedAddition will alter the aim's screen position when entering the given radius.
 *	   
 *	   Then, a parameter Strength is used to interpolate these two types of offsets.
 * 
 *	   This strategy can be particularly useful when your aim target is high above the ground
 *	   and the pitch could be very large. 
 *	
 *	   This extension can be used along with the ConstrainPitch extension.
 * 
 * NOTE: this extension can ONLY be used in lock-on cameras, i.e., ScreenFollow & TargetingAim.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UModifyAimPointExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UModifyAimPointExtension();

protected:
	/** Radius within which the aim point will be pushed away. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyAimPoint", meta = (ClampMin = "0.1"))
	float Radius;

	/** Strength of the dynamic addition, generally, a value between 0.9 and 0.95 performs well. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyAimPoint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Strength;

private:
	UFramingFollow* ScreenFollowComponent;
	UTargetingAim* TargetingAimComponent;

	bool bInModify;
	float CachedPitch;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

};
