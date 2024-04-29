// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Curves/CurveFloat.h"
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
	/** The speed of rolling. Different strategies: (1) Manual, (2) constant, (3) proportional to velocity. 
	  * Note that if you use Manual, you have full control over roll, regardless of all the parameters here, e.g., MaxRoll and MinVelocity.
	  * Use GetFollowTargetVelocity() to access the follow target's velocity if you need it.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VelocityBasedRollingExtension")
	ERollScheme RollScheme;

	/** Whether to inverse the rolling direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	bool bInverse;

	/** Max roll allowed. Must be positive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "90.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float MaxRoll;

	/** Minimum velocity to start rolling. Note that veloccity is the projected one onto camera's local right/left vector. Must be positive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "100000.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float MinVelocity;

	/** Will not start rolling unitl this time period has been arrived at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "10.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float RollWaitTime;

	/** Rolling speed when RollScheme is Constant. Unit: degree/second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", EditCondition = "RollScheme == ERollScheme::Constant"))
	float RollSpeed;

	/** Rolling speed ratio w.r.t. follow target's velocity when RollScheme is ProportionalToVelocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "-10.0", ClampMax = "10.0", EditCondition = "RollScheme == ERollScheme::ProportionalToVelocity"))
	float RollSpeedRatio;

	/** Rolling multiplier applied to the roll increment. Default is 1.0 for all x-axis values (current roll). If ExternalCurve is provided, will use it.
	  * Use this curve to control the tangent/smoothness when rolling proceeds.
	  * A good practice is to set a relatively small value when rolling ends to perform a smooth blend-out rolling effect.
	  * The value should not be too small when rolling starts.
	  * Used when RollScheme is Constant or ProportionalToVelocity.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	FRuntimeFloatCurve RollMultiplierCurve;

	/** Damping when rolling. Small values indicate more responsiveness. Will be applied after RollMultiplierCurve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float RollDamping;

	/** Will not restore from rolling unitl this time period has been arrived at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", ClampMax = "10.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float RestoreWaitTime;

	/** Restore speed. Unit: degree/second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float RestoreSpeed;

	/** Rolling multiplier applied to the roll increment. Default is 1.0 for all x-axis values (current roll). If ExternalCurve is provided, will use it.
	  * Use this curve to control the tangent/smoothness when restoring proceeds.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	FRuntimeFloatCurve RestoreMultiplierCurve;

	/** Damping when restoring from rolling. Small values indicate more responsiveness. Applied after RestoreMultiplierCurve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VelocityBasedRollingExtension", meta = (ClampMin = "0.0", EditCondition = "RollScheme == ERollScheme::Constant || RollScheme == ERollScheme::ProportionalToVelocity"))
	float RestoreDamping;

public:
	/** Indicating whether camera is rolling, even if it's restoring. */
	bool bRolling;

	/** Elapsed roll wait time. */
	float ElapsedRollWaitTime;

	/** Elapsed restore wait time. */
	float ElapsedRestoreWaitTime;

	/** Current roll. */
	float CurrentRoll;

	/** Async action for AsyncStartRoll. */
	class FERollDelayAction* Action;

	/** The following variables are used when RollScheme is Manual and StartRoll is called. */
	float MannualOriginRoll;
	float MannualTargetRoll;
	float MannualElapsedTime;
	float MannualDuration;
	float MannualBlendExp;
	TEnumAsByte<EEasingFunc::Type> MannualBlendFunc;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Reset camera roll before tick begins. */
	virtual void BindToOnPreTickComponent() override;

	/** Async start rolling from current roll. Used when RollScheme is Manual. You can use AsyncPauseRoll to pause or unpause rolling.
	  * If you just want to normally start rolling, use the StartRoll blueprint node. 
	  * DO NOT mix-use these two nodes.
	  * @param TargetRoll - Target roll, starting from current roll.
	  * @param Duration - Rolling duraion, default is 1.0.
	  * @param BlendFunc - Function to control the smoothness of rolling.
	  * @param BlendeXp - Blending exponential.
	  */
	UFUNCTION(BlueprintCallable, Category = "VelocityBasedRollingExtension", meta = (Latent, LatentInfo = "LatentInfo", Duration = "1.0", BlendExp = "2.0"))
	void AsyncStartRoll(float TargetRoll, float Duration, TEnumAsByte<EEasingFunc::Type> BlendFunc, float BlendExp, struct FLatentActionInfo LatentInfo);

	/** Pause or unpause rolling for AsyncStartRoll. Used when RollScheme is Manual. */
	UFUNCTION(BlueprintCallable, Category = "VelocityBasedRollingExtension")
	void AsyncPauseRoll(bool bSetPause);

	/** Start rolling. Used when RollScheme is Manual. You can use the AsyncStartRoll node to do something when rolling completes. Cannot be paused.
	  * If you just want to normally start rolling, use this node instead. 
	  * DO NOT mix-use these two nodes.
	  * @param TargetRoll - Target roll, starting from current roll.
	  * @param Duration - Rolling duraion, default is 1.0.
	  * @param BlendFunc - Function to control the smoothness of rolling.
	  * @param BlendeXp - Blending exponential.
	  */
	UFUNCTION(BlueprintCallable, Category = "VelocityBasedRollingExtension", meta = (Duration = "1.0", BlendExp = "2.0"))
	void StartRoll(float TargetRoll, float Duration, TEnumAsByte<EEasingFunc::Type> BlendFunc, float BlendExp);

	/** Get follow target's world-space velocity. Must check the returned FollowTarget's validity before using the velocity.
	  * Will return zero vector if the follow target is inavlid.
	  */
	UFUNCTION(BlueprintCallable, Category = "VelocityBasedRollingExtension")
	FVector GetFollowTargetVelocity(AActor*& FollowTarget);

private:
	/** Initialize RollMultiplierCurve and RestoreMultiplierCurve. */
	void InitRollAndRestoreMultiplierCurve();

	/** Add roll to camera. */
	void AddRoll(const float& DeltaTime, const float& CurrentVelocity);
	
	/** Remain current roll. */
	void RemainRoll();

	/** Restore roll. */
	void RestoreRoll(const float& DeltaTime);

	/** Reset variables when roll is small. */
	bool ResetWhenRollIsSmall();
};

