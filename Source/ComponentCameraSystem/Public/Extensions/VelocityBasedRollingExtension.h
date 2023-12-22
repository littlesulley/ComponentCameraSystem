// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "VelocityBasedRollingExtension.generated.h"

/**
 * This extension is used to add roll to camera based on the FOLLOW TARGET's velocity.
 * You should make sure the follow target is valid.
 * Generally this component should be applied at the last step of the camera pipeline.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UVelocityBasedRollingExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UVelocityBasedRollingExtension();

protected:
	/** Max roll allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxRoll;

	/** Minimum velocity to start rolling. Note that veloccity the projected one on camera's local right/left vector. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "100000.0"))
	float MinVelocity;

	/** Will not start rolling unitl this time period has been arrived at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RollWaitTime;

	/** Will not restore from rolling unitl this time period has been arrived at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RestoreWaitTime;

	/** The speed of rolling. Different strategies: (1) constant, (2) propotional to velocity.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension")
	ERollScheme RollSceheme;

	/** Rolling speed when RollSceheme is Constant. Unit: degree/second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", EditCondition = "RollSceheme == ERollScheme::Constant"))
	float RollSpeed;

	/** Rolling speed ratio w.r.t. follow target's velocity when RollSceheme is ProportionalToVelocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "-10.0", ClampMax = "10.0", EditCondition = "RollSceheme == ERollScheme::ProportionalToVelocity"))
	float RollSpeedRatio;

	/** Damping when rolling. Small values indicate more responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0"))
	float RollDamping;

	/** Restore speed. Unit: degree/second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0"))
	float RestoreSpeed;

	/** Damping when restoring from rolling. Small values indicate more responsiveness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0"))
	float RestoreDamping;

private:
	/** Indicating whether camera is rolling, even if it's restoring. */
	bool bRolling;

	/** Elapsed roll wait time. */
	float ElapsedRollWaitTime;

	/** Elapsed restore wait time. */
	float ElapsedRestoreWaitTime;

	/** Current roll. */
	float CurrentRoll;

	/** Follow target's location at previous location. */
	FVector CachedFollowTargetLocation;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Reset camera roll before tick begins. */
	virtual void BindToOnPreTickComponent() override;

private:
	/** Add roll to camera. */
	void AddRoll(const float& DeltaTime, const float& CurrentVelocity);
	
	/** Remain current roll. */
	void RemainRoll();

	/** Restore roll. */
	void RestoreRoll(const float& DeltaTime);

	/** Reset variables when roll is small. */
	bool ResetWhenRollIsSmall();
};
