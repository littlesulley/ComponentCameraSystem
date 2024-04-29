// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Utils/ECameraTypes.h"
#include "ResolveOcclusionExtension.generated.h"


/**
 * ResolveOcclusionExtension is used for camera occlusion detection along the direction from camera to aim target.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UResolveOcclusionExtension : public UECameraExtensionBase
{
	GENERATED_BODY()

public:
	UResolveOcclusionExtension();

protected:
	/** A set of parameters describing how to avoid occlusion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveOcclusionExtension")
	FOccluderParams OccluderParams;

	/** Ignore obstacles that are less than this distance from target. Set this value as small as possible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveOcclusionExtension", meta = (ClampMin = "0.0"))
	float MinimumDistanceFromTarget;

	/** Preserve a fixed camera distance to occlusion. Zero is recommended. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveOcclusionExtension", meta = (ClampMin = "0.0"))
	float CameraDistanceFromOcclusion;

	/** Damping when avoiding occlusion. Small values indicate more responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveOcclusionExtension", meta = (ClampMin = "0.0"))
	float OcclusionDamping;

	/** Damping when restoring from occlusion. Small values indicate more responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResolveOcclusionExtension", meta = (ClampMin = "0.0"))
	float RestoreDamping;

	/** How long occlusion has lasted? */
	float OcclusionElapsedTime;
	/** Cached raw camera location. */
	FVector CachedRawLocation;
	/** Delta distance from camera. */
	float DeltaDistanceFromCamera;
	/** Already damped time. */
	float AlreadyDampedTime;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Reset camera location before tick begins for correct occlusion damping. */
	virtual void BindToOnPreTickComponent() override;

	/** Reset variables and do restoring damping. */
	void ResetVariablesAndRestoreDamping(float DeltaTime, const float& Input, float Damping, const FVector& Direction);

	/** Reset variables if needed. */
	void ResetVariables()
	{
		OcclusionElapsedTime = 0.0f;
		AlreadyDampedTime = 0.0f;
	}
};