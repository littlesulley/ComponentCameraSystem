// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/ECameraDamper.h"
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
	/** Additional offset applied to the aim target. This can be modified by other components. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "TargetingAim")
	FVector AdditionalAimOffset;

	/** Damper you want to use for damping. If this is None, no damping will be applied. */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "TargetingAim")
	TObjectPtr<UECameraRotatorDamper> Damper;

	/** Screen space offset applied to the *real* aim target after applying AimOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-0.5", ClampMax = "0.5"))
	FVector2f ScreenOffset;

	/** Width of the screen offset. Pivoted at the X axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2f ScreenOffsetWidth;

	/** Height of the screen offset. Pivoted at the Y axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetingAim", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2f ScreenOffsetHeight;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;

	/** Get original aim position, without AdditionalAimOffset. */
	FVector GetAimPosition() { return UECameraComponentAim::GetRealAimPosition(true); }

	/** Get the *real* aim position, based on world space. */
	virtual FVector GetRealAimPosition(bool bWithOffset) override { return UECameraComponentAim::GetRealAimPosition(bWithOffset) + AdditionalAimOffset; }

	/** Get screen offset. */
	FVector2f GetScreenOffset() { return ScreenOffset; }

	/** Get screen offset width. */
	FVector2f GetScreenOffsetWidth() { return ScreenOffsetWidth; }

	/** Get screen offset height. */
	FVector2f GetScreenOffsetHeight() { return ScreenOffsetHeight; }

	/** Set additional aim offset. */
	void SetAdditionalAimOffset(FVector InOffset) { AdditionalAimOffset = InOffset; }

	/** Set additional aim offset. */
	void SetAdditionalAimOffset(float InX = 0, float InY = 0, float InZ = 0) { AdditionalAimOffset.X = InX, AdditionalAimOffset.Y = InY, AdditionalAimOffset.Z = InZ; }

	/** Check if camera is too close to the aim target. */
	bool CheckIfTooClose(const FVector& AimPosition);

	/** Set delta rotation. */
	void SetDeltaRotation(const FVector& AimPosition, FRotator& TempDeltaRotation);

	/** Damp delta rotation. */
	FRotator DampDeltaRotation(const FRotator& TempDeltaRotation, float DeltaTime, const FVector& AimPosition);

	/** Ensure after damping, the aim target will be within the bound. */
	void EnsureWithinBounds(FRotator& DampedDeltaRotation, const FVector& AimPosition);
};
