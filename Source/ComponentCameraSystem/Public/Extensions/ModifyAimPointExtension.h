// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "ModifyAimPointExtension.generated.h"

class UScreenFollow;
class UTargetingAim;

/**
 * This extension can also be used to constrain pitch, but in a different way:
 *     When the follow position is within a given radius of the aim position,
 *	   the aim position will be pushed away from the camera, ensuring camera jitters won't happen.
 *	   This is achieved by interpolating between three types of additional aim offsets, which you can see 
 *	   from the read-only property AdditionalAimOffset in the TargetingAim component.
 *     1) The first type is PitchAddition, which tries to keep a fixed pitch angle between the follow position
 *     and the aim position after applied with this kind of aim offset.
 *     2) The second type is CamToAimAddition, which tries to keep a fixed aim position in screen space,
 *	   by extending the aim position along the direction from camera to the aim target.
 *	   3) The third type is CamForwardAddition, which extends the aim position along the camera's forward direction.
 *	   
 *	   Then, a parameter Weights is used to interpolate these types of offsets.
 * 
 *	   This strategy can be particularly useful when your aim target is high above the ground
 *	   and the pitch could be very large. 
 *	   
 *     This extension can be used along with the ConstrainPitch extension.
 * 
 *	   Blog post: https://sulley.cc/2023/10/24/22/31/
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

	/** Weight for each type of aim offset. X, Y and Z respectively correspond to PitchAddition, CamToAimAddition and CamForwardAddition. 
	 *  You should make sure the sum of X, Y and Z is equal to or greater than 1.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyAimPoint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FVector Weights;

private:
	UScreenFollow* ScreenFollowComponent;
	UTargetingAim* TargetingAimComponent;

	bool bInModify;
	float CachedPitch;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

};
