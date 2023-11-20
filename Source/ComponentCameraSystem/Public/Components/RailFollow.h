// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentFollow.h"
#include "RailFollow.generated.h"

class ACameraRig_Rail;

/**
 * RailFollow component enforces camera to move along a given rail trajectory.
 * Its position on the rail is computed to get closer to the follow target, or by a fixed speed.
 * If you find camera moves very fast at the beginning of play, check whether the BlendTime parameter 
 * in the CallCamera node is non-zero.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API URailFollow : public UECameraComponentFollow
{
	GENERATED_BODY()
	
public:
	URailFollow();

protected:
	/** The rail along which camera moves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow")
	TSoftObjectPtr<ACameraRig_Rail> Rail;

	/** Determines whether the orientation of the camera should be in the direction of the rail. 
	 *  You should not specify AimComponent if you are enabling this.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow")
	bool bLockOrientationToRail;

	/** How would you like the camera to move on rail. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow")
		
	ERailFollowType FollowType;

	/** Follow damping when FollowType is FollowTarget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (ClampMin = "0.0", EditCondition = "FollowType == ERailFollowType::FollowTarget"))
	float Damping;

	/** Max velocity (absolute value) when FollowType is FollowTarget.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (ClampMin = "0.0", EditCondition = "FollowType == ERailFollowType::FollowTarget"))
	float MaxVelocity;

	/** Start position when FollowType is FixedSpeed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "FollowType == ERailFollowType::FixedSpeed"))
	float StartPosition;

	/** Duration when FollowType is FixedSpeed. Set as 0 for infinite move. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (ClampMin = "0.0", EditCondition = "FollowType == ERailFollowType::FixedSpeed"))
	float Duration;

	/** Speed (per second) when FollowType is FixedSpeed, in percentage of the overall track length. Can be negative. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (ClampMin = "-1.0", ClampMax = "1.0", EditCondition = "FollowType == ERailFollowType::FixedSpeed"))
	float Speed;

	/** Whether to loop when FollowType is FixedSpeed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (EditCondition = "FollowType == ERailFollowType::FixedSpeed"))
	bool bLoop;

	/** A set of key points on rail when FollowType is Manual.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow", meta = (EditCondition = "FollowType == ERailFollowType::Manual"))
	FRailManualParams ManualKeyPoints;

	/** Whether to restart from scratch when resuming this component. Default is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RailFollow")
	bool bResetOnResume;


	/** Whether the camera is currently manually positioned using blueprint. */
	bool bIsBlueprinting;
	/** Whether camera moves forward when is blueprinting. */
	bool bIsForwardWhenIsBlueprinting;
	/** Start position on rail when is blueprinting. */
	float StartPositionWhenIsBlueprinting;
	/** Target position on rail when is blueprinting. */
	float TargetPositionWhenIsBlueprinting;
	/** Blend time when is blueprinting. */
	float BlendTimeWhenIsBlueprinting;
	/** Blend function when is blueprinting. */
	TEnumAsByte<EEasingFunc::Type> BlendFuncWhenIsBlueprinting;
	/** Elasped time when is blueprinting. */
	float ElapsedTimeWhenIsBlueprinting;

	/** Previous position on rail. */
	float PreviousPositionOnRail;
	/** Current position on rail. */
	float CurrentPositionOnRail;
	/** Elasped time since start, for FixedSpeed mode. */
	float ElapsedTime;
	/** Elasped blend time, for Manual mode. */
	float ElapsedBlendTime;
	/** Elasped duration time, for Manual mode. */
	float ElapsedDurationTime;
	/** Current key point, for Manual mode. */
	int CurrentKeyPoint;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Get camera position on rail. Code is basically copied from CameraRig_Rail.cpp. */
	FTransform UpdateTransformOnRail();

	/** Set rail position and update camera transform. */
	void SetRailPositionAndUpdateCameraTransform(float Position);

	/** Get normalized position on rail nearest to target */
	float GetNormalizedPositionOnRailNearestToTarget(AActor* Target);

	/** Get damped delta location from current location to desired position on rail. */
	FVector GetDampedDeltaLocationInWorldSpace(float DeltaTime, float DampingTime, FTransform& DesiredTransform);

	/** Get damped delta position on rail. */
	float GetDampedDeltaPositionOnRail(float Start, float End, bool bIsForward, float DeltaTime, float DampingTime);

	/** Get amount camera should move. */
	float GetMoveAmount(float Start, float End, bool bIsForward);

	/** Apply a set of restrictions to the delta position on rail, e.g., max velocity. */
	void ApplyPostRestrictions(const float& DeltaTime, float& DeltaPosition);

	/** Camera moves when bIsBlueprinting is true. */
	void ResolveWhenIsBlueprinting(float DeltaTime);

public:
	/** Hard set camera position on rail with damping.
	 * @param Position - A floating number in [0,1] representing the position on rail.
	 * @param BlendTime - A non-negative floating number representing the duration to get to Position.
	 * @param BlendFunc - Blend function.
	 * @param IsForward - Blending forward or backward from here to Position.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|RailFollow", meta = (DisplayName = "SetPositionOnRailWithDamping", WorldContext = "WorldContextObject"))
	void SetPositionOnRailWithDamping(const UObject* WorldContextObject, float Position, float BlendTime, TEnumAsByte<EEasingFunc::Type> BlendFunc, bool bIsForward);
};
