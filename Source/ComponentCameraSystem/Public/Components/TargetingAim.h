// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentAim.h"
#include "TargetingAim.generated.h"

/**
 * TargetingAim makes camera aim at a target, with offset and damping.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UTargetingAim : public UECameraComponentAim
{
	GENERATED_BODY()

public:
	UTargetingAim();

protected:
	/** Whether to use local space rotation. This is currently experimental, do not use it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim")
	bool bLocalRotation;

	/** World space position offset applied to the aim target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim")
	FVector AimOffset;

	/** Damp parameters you want to use for damping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim")
	FDampParams DampParams;

	/** Screen space offset applied to the *real* aim target after applying AimOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-0.5", ClampMax = "0.5"))
	FVector2D ScreenOffset;

	/** Width of the screen offset. Pivoted at the X axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2D ScreenOffsetWidth;

	/** Height of the screen offset. Pivoted at the Y axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2D ScreenOffsetHeight;

	FVector RealAimPosition;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;

	/** Get the *real* aim position, based on world space. */
	virtual FVector GetRealAimPosition() override { return AimTarget->GetActorLocation() + AimOffset; }

	/** Get the real aim position. */
	FVector GetAimPosition() { return RealAimPosition; }

	/** Get screen offset. */
	FVector2D GetScreenOffset() { return ScreenOffset; }

	/** Get screen offset width. */
	FVector2D GetScreenOffsetWidth() { return ScreenOffsetWidth; }

	/** Get screen offset height. */
	FVector2D GetScreenOffsetHeight() { return ScreenOffsetHeight; }

	/** Check if camera is too close to the aim target. */
	bool CheckIfTooClose(const FVector& AimPosition);

	/** Set delta rotation. */
	void SetDeltaRotation(const FVector& AimPosition, FRotator& TempDeltaRotation);

	/** Damp delta rotation. */
	FRotator DampDeltaRotation(const FRotator& TempDeltaRotation, float DeltaTime, const FVector& AimPosition);

	/** Ensure after damping, the aim target will be within the bound. */
	void EnsureWithinBounds(FRotator& DampedDeltaRotation, const FVector& AimPosition);
};
