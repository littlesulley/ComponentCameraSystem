// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "ResolveGroupActorExtension.generated.h"

/**
 * This extension will adjust camera FOV or (and) distance to encapsulate a bounding box formed by
 * the group actor, if any in follow component or aim component. This bounding box is the union of
 * several sub bounding boxes defined by the widths and heights of target actors composing the group actor.
 * You must be careful of the Stage property of this extension, as this property may vary in terms of 
 * the stage at which the group actor is applied. In general case, you should use PostAim.
 */
UCLASS()
class COMPONENTCAMERASYSTEM_API UResolveGroupActorExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UResolveGroupActorExtension();

protected:
	/** The method you want to use to resolve group actor in screen space. */
	UPROPERTY(EditAnywhere, Category = "ResolveGroupActorExtension")
	EResolveGroupActorMethod ResolveMethod;

	/** The minimum and maximum FOV. Minimum should generally be the default FOV, in most cases it is 90. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveGroupActorExtension", meta = (EditCondition = "ResolveMethod != EResolveGroupActorMethod::DistanceOnly"))
	FVector2D FOVRange;

	/** Time used to damp FOV. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveGroupActorExtension", meta = (EditCondition = "ResolveMethod != EResolveGroupActorMethod::DistanceOnly"))
	float FOVDampTime;

	/** The minimum and maximum displaceable camera distance. Distance is the Z-axis distance in camera's local space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveGroupActorExtension", meta = (EditCondition = "ResolveMethod != EResolveGroupActorMethod::ZoomOnly"))
	FVector2D DistanceRange;

	/** Time used to damp distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveGroupActorExtension", meta = (EditCondition = "ResolveMethod != EResolveGroupActorMethod::ZoomOnly"))
	float DistanceDampTime;

	/** Which percent of the COMBINED (the union of all sub-bounding boxes) bounding box will be tolerated. 
	 *  0 means totally conforming to the bounding box.
	 *  1 means the bounding box will be fully ignored, in which case FOV and distance will never be adjusted.
	 *  0.5 means the bounding box will shrink by 50% at its size.
	 *  Increasing this value may make the target actors move beyond screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveGroupActorExtension", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Tolerance;
	
	/** Already adjusted distance. */
	float AdjustedDistance;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;

	/** Resolve group actor in screen space according to ResolveMethod. */
	void ResolveGroupActor(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime);
	/** Get required FOV that encapsulates target actors in front of camera. */
	float GetFOVForZoomOnly(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime);
	float GetDistanceForDistanceOnly(TArray<FBoundingWrappedActor> TargetActors, float DeltaTime, const float& FOV);
};
