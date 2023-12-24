// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentFollow.h"
#include "CraneFollow.generated.h"

class ACameraRig_Crane;

/**
 * RailFollow component enforces camera to move along a given crane.
 * You can adjust the crane arm length, crane yaw and crane pitch to control camera movement.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UCraneFollow : public UECameraComponentFollow
{
	GENERATED_BODY()

public:
	UCraneFollow();
	
protected:
	/** The crane along which camera moves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow")
	TSoftObjectPtr<ACameraRig_Crane> Crane;

	/** Determines whether the camera should be oriented outwards in the direction from crane origin to camera position.
	 *  You should not specify AimComponent if you are enabling this.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow")
	bool bLockOrientationOutwards;

	/** How would you like the camera to move on crane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow")
	ECraneFollowType FollowType;

	/** Whether to lock arm length when FollowType is FollowTarget. You should be careful of the start arm length when enabling this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::FollowTarget"))
	bool bLockArmLength;

	/** Whether to lock crane yaw when FollowType is FollowTarget. You should be careful of the start yaw when enabling this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::FollowTarget"))
	bool bLockYaw;

	/** Whether to lock crane pitch when FollowType is FollowTarget. You should be careful of the start pitch when enabling this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::FollowTarget"))
	bool bLockPitch;

	/** Start position when FollowType is FixedSpeed or FollowTarget. Respectively for arm length, crane yaw and crane pitch, in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::FixedSpeed || FollowType == ECraneFollowType::FollowTarget"))
	FVector StartPosition;

	/** Duration when FollowType is FixedSpeed. Set as 0 for infinite move. Respectively for arm length, crane yaw and crane pitch, in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (ClampMin = "0.0", EditCondition = "FollowType == ECraneFollowType::FixedSpeed"))
	FVector Duration;

	/** Speed (per second) when FollowType is FixedSpeed. Respectively for arm length, crane yaw and crane pitch, in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::FixedSpeed"))
	FVector Speed;

	/** A set of key points on crane when FollowType is Manual.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow", meta = (EditCondition = "FollowType == ECraneFollowType::Manual"))
	FCraneManualParams ManualKeyPoints;
	
	/** Whether to restart from scratch when resuming this component. Default is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CraneFollow")
	bool bResetOnResume;

	/** Whether the camera is currently manually positioned using blueprint. */
	bool bIsBlueprinting;

	/** If has begun. */
	bool bHasBegun;
	/** Elasped time since start, for FixedSpeed mode. */
	float ElapsedTime;
	/** Current key point for Manual mode. */
	int CurrentKeyPoint;
	/** Elapsed blend time, for Manual mode. */
	float ElapsedBlendTime;
	/** Elapsed duration time, for Manual mode. */
	float ElapsedDurationTime;

	/** Crane location, for HUD. */
	FVector CraneLocation;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;

	/** Get crane location. */
	FVector GetCraneLocation() { return CraneLocation; }
	
	/** Get camera position on crane. */
	FTransform UpdateTransformOnCrane();

	/** Update and set camera transform. */
	void UpdateAndSetTransform();

	/** Set new crane position according to follow position. */
	void SetPositionToFollow(const FVector& FollowPosition, float DeltaTime);

	/** Renormalize yaw and pitch, respectively to [-180, 180]. */
	void RenormalizeYawAndPitch(FVector& Input);

	/** Get move amount for arm length, yaw and pitch, in world space. */
	FVector GetMoveAmount(const FVector& Start, const FVector& End, bool bForwardYaw, bool bForwardPitch);

	void ResolveWhenIsBlurprinting(float DeltaTime);
};
