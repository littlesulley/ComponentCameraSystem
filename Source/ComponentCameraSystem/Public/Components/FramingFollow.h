// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/ECameraLibrary.h"
#include "Components/ECameraComponentFollow.h"
#include "FramingFollow.generated.h"

class UControlAim;

/**
 * FramingFollow keeps a fixed position of the follow target on screen space.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UFramingFollow : public UECameraComponentFollow
{
	GENERATED_BODY()

public:
	UFramingFollow();

protected:
	/** Camera distance to the *real* follow target after applying FollowOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	float CameraDistance;

	/** A curve used to adjust camera distance according to camera pitch. The x-axis range should be within [-90, 90]. 
	 *  When camera looks down, pitch is negative; when camera looks up, pitch is positive.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	UCurveFloat* PitchDistanceCurve;

	/** Follow target's local space offset applied to the follow target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	FVector FollowOffset;

	/** Whether to adapt camera orientation to character movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	bool bAdaptToMovement;

	/** Speed when adapting camera orientation to character movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	float AdaptToMovementSpeed;

	/** Damp parameters you want to use for damping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	FDampParams DampParams;

	/** Screen space offset applied to the *real* follow target after applying FollowOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow", meta = (ClampMin = "-0.5", ClampMax = "0.5"))
	FVector2D ScreenOffset;

	/** Start and end distance in which adaptive screen offset (X axis) is enable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	FVector2D AdaptiveScreenOffsetDistanceX;

	/** Start and end distance in which adaptive screen offset (Y axis) is enable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow")
	FVector2D AdaptiveScreenOffsetDistanceY;

	/** Width of the screen offset. Pivoted at the X axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2D ScreenOffsetWidth;

	/** Height of the screen offset. Pivoted at the Y axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FramingFollow", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2D ScreenOffsetHeight;

	/** Real follow position. */
	FVector RealFollowPosition;
	/** Local space follow position. */
	FVector LocalFollowPosition;
	/** Cached delta residual. For some damping algorithm. */
	FVector DeltaResidual;
	/** Previous camera residual. For some damping algorithms. */
	FVector PreviousResidual;
	/** Location at previous frame. Used for Spring damp. */
	FVector PreviousLocation;
	/** Velocity for exact spring. */
	FVector ExactSpringVel;
	/** Cached ControlAim component. */
	UControlAim* ControlAim;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Return real follow position. */
	FVector GetFollowPosition() { return RealFollowPosition; }

	/** Get local space follow position. */
	FVector GetLocalSpaceFollowPosition() { return LocalFollowPosition; }

	/** Get screen offset. */
	FVector2D GetScreenOffset() { return ScreenOffset; }

	/** Get screen offset width. */
	FVector2D GetScreenOffsetWidth() { return ScreenOffsetWidth; }

	/** Get screen offset height. */
	FVector2D GetScreenOffsetHeight() { return ScreenOffsetHeight; }

	/** Get the *real* screen offset based on the distance between follow position and aim position. */
	FVector2D GetAdaptiveScreenOffset(const FVector& FollowPosition, const FVector& AimPositio);

	/** Normalize pitch into [-90, 90]. */
	float NormalizePitch(float Pitch);

	/** Set delta position along the local X axis. */
	void SetForwardDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, float RealCameraDistance);

	/** Get delta position along the local YZ plane. */
	void SetYZPlaneDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, const FVector2D& RealScreenOffset);

	/** Damp temporary delta position. */
	FVector DampDeltaPosition(const FVector& LocalSpaceFollowPosition, const FVector& SpringTemporalInput, const FVector& TempDeltaPosition, float DeltaTime, const FVector2D& RealScreenOffset);

	/** Ensure after damping, the follow target will be within the bound. */
	void EnsureWithinBounds(const FVector& LocalSpaceFollowPosition, FVector& DampedDeltaPosition, const FVector2D& RealScreenOffset);

	/** Check whether has mouse input. */
	bool HasControlAimInput();
};

