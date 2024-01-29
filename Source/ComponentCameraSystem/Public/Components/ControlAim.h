// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentAim.h"
#include "ControlAim.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

/**
 * Player input camera rotation.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UControlAim : public UECameraComponentAim
{
	GENERATED_BODY()
	
public:
	UControlAim();

protected:
	/** Whether you are using EnhancedInput. If true, the input will read from enhanced input. Otherwise, it reads from mouse input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ControlAim")
	bool bEnhancedInput;

	/** The input action to consume camera rotation. Ensure this action is correct. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ControlAim", meta = (EditCondition = "bEnhancedInput == true"))
	class UInputAction* LookAction;

	/** Whether to consume input. If set to false, will not consume input from player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	bool bConsumeInput;

	/** Aim assist settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	FAimAssist AimAssist;

	/** Recentering parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	FRecenteringParams RecenteringParams;

	/** Horizontal heading to which horizontal range is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	EHeading HorizontalHeading;

	/** Hard specified horizontal forward. Will ignore its pitch angle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (EditCondition = "HorizontalHeading == EHeading::HardForward"))
	FVector HorizontalHardForward;

	/** Source actor for horizontal soft forward. Its position is used as the start position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (EditCondition = "HorizontalHeading == EHeading::SoftForward"))
	AActor* HorizontalSource;

	/** Destination actor horizontal for soft forward. Its position is used as the end position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (EditCondition = "HorizontalHeading == EHeading::SoftForward"))
	AActor* HorizontalDestination;

	/** Camera yaw bounding range, within [-180,180]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	FVector2D HorizontalRange;

	/** Whether to wrap camera horizontal rotation. Default is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	bool bWrap;

	/** Camera pitch bounding range, within (-90, 90). Will be overridden if using OrbitFollow.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	FVector2D VerticalRange;

	/** Yaw move speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "0.0"))
	float HorizontalSpeed;

	/** Pitch move speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "0.0"))
	float VerticalSpeed;

	/** Acceleration and deceleration time when changing yaw. First element is acceleration, second is deceleration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "0.0"))
	FVector2D HorizontalDamping;

	/** Acceleration and deceleration time when changing pitch. First element is acceleration, second is deceleration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim", meta = (ClampMin = "0.0"))
	FVector2D VerticalDamping;

	/** Whether to replicate camera's rotation to Controller. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	bool bSyncToController;

	/** Whether to replicate camera's yaw to aim target. This is useful when you want to set character's rotation in synchronization with this camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ControlAim")
	bool bSyncYawToAimTarget;

	/** This frame's raw mouse input delta X and Y. */
	float RawMouseDeltaX;
	float RawMouseDeltaY;
	/** Cached scaled mouse delta X and Y. */
	float CachedMouseDeltaX;
	float CachedMouseDeltaY;

	/** Recentering already wait time. */
	float RecenteringWaitElaspedTime;
	/** If is manually recentering. */
	bool bManualRecentering;
	/** Duration of manual recentering. */
	float ManualRecenteringDuration;
	/** Elapsed time of manual recentering. */
	float ManualRecenteringElapsedTime;
	/** Manual recentering function. */
	TEnumAsByte<EEasingFunc::Type> ManualRecenteringFunc;
	/** Manual recentering exponential. */
	float ManualRecenteringExp;
	/** Manual recentering original quaternion. */
	FQuat ManualRecenteringOriQuat;
	/** Manual recentering target quaternion. */
	FQuat ManualRecenteringTargetQuat;

	/** Cached world follow position for rectification. */
	FVector WorldFollowPosition;
	/** Cached local follow position for rectification. */
	FVector LocalFollowPosition;

	/** If currently is in aim assist. */
	bool bInAimAssist;
	/** Elapsed time since last target actors update. */
	float ElapsedUpdateTime;
	/** Screen distance when in aim assist. */
	float ScreenDistanceInAimAssist;
	/** Output target actors. */
	TMap<FVector, TArray<AActor*>> OutActors;
	/** The actor that is in aim assist. */
	AActor* ActorInAimAssist;
	/** Offset. */
	FVector OffsetInAimAssist;

	/** Enhanched input subsystem. */
	UEnhancedInputLocalPlayerSubsystem* Subsystem;
	
public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Get input mouse delta. */
	void GetMouseDelta();

	/** Get the multiplication of raw input mouse delta X and HorizontalSpeed. */
	UFUNCTION(BlueprintPure, Category = "ECamera|ControlAim")
	float GetMouseDeltaX() { return RawMouseDeltaX * HorizontalSpeed; }
	/** Get the multiplication of raw input mouse delta Y and VerticalSpeed. */
	UFUNCTION(BlueprintPure, Category = "ECamera|ControlAim")
	float GetMouseDeltaY() { return RawMouseDeltaY * VerticalSpeed; }

	/** Manually starts recentering. Used when recentering scheme is Manual, otherwise this function does nothing. 
	  * If you would like to use this node when scheme is Auto, you should first set the scheme to Manual.
	  * Once recentering completes, the camera will stop recentering to the heading direction, e.g., TargetForward.
	  * Note that this function does not block player input. If player input is consumed during recentering, recentering will halt.
	  * In this case, you may want to set bConsumeInput to false.
	  * Besides, this function will determine the target heading direction once it's called. This means even if the heading direction
	  * changes during recentering, this function will not use the new heading direction.
	  * 
	  * @param Duration - Recentering duration.
	  * @param Func - Recentering function.
	  * @param Exp - Recentering exponential.
	  */
	UFUNCTION(BlueprintCallable, Category = "ECamera|ControlAim")
	void StartRecentering(float Duration, TEnumAsByte<EEasingFunc::Type> Func, float Exp);

	/** Immediately stop recentering. Used when recentering scheme is Manual. This function may be called when the follow target moves. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|ControlAim")
	void StopRecentering();

	/** Return bInAimAssist for HUD. */
	bool GetInAimAssist() { return bInAimAssist; }

	/** Return AimAsset for HUD. */
	const FAimAssist& GetAimAssist() { return AimAssist; }

	/** Get damped mouse delta. */
	float GetDampedMouseDelta(const float& MouseDelta, bool bIsHorizontal, const float& DeltaTime);

	/** Resolve recentering. Returns whether camera is currently recentering. */
	bool ResolveRecentering(const float& DeltaTime);

	/** Get recentering target quaternion. */
	FQuat GetRecenteringTargetQuat();

	/** Convert yaw into range (-180, 180]. */
	float ConvertYaw(float Yaw);

	/** Constrain yaw. Returns the additional yaw when triggering wrap. */
	float ConstrainYaw(float& ResultDeltaX);

	/** Constrain pitch. */
	void ConstrainPitch(float& ResultDeltaY);

	/** Check if there exists an actor will be in aim assist. */
	bool CheckAimAssist();

	/** Recitify camera position after rotation so that the follow target maintains its position. */
	void RectifyCameraPosition();

	/** Check if the follow target is moving inwards to the screen and recentering heading is TargetForward. */
	bool CheckIfPauseRecenteringWhenTargetForward(const FQuat& TargetQuat);
};
