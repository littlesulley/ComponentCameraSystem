// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ECameraTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "ECameraLibrary.generated.h"

class AEPlayerCameraManager;
class AESequencedCameraSetupActor;

/**
 * UCameraLibrary contains functions for blueprint callable nodes.
 */
UCLASS()
class COMPONENTCAMERASYSTEM_API UECameraLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static int Factorials[];

public:
	/** Collectively damp a vector using different damp methods with the same damp time.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input vector to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output vector.
	 * @param (Spring) TemporalInput - Camera `current position` - `previous position`
	 * @param (Spring) SpatialInput - Camera `desired position` - `current position`
	 * @param (ExactSpring) CurrentVelocity - Input vector representing current velocity.
	 * @param (ExactSpring) TargetVector - Input vector representing target property, e.g., position.
	 * @param (ExactSpring) TargetVelocity - Input vector representing target velocity.
	 * @param (ExactSpring) OutVelocity - Ouput velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampVectorWithSameDampTime(const FDampParams DampParams,   // All methods
											   const FVector Input,            // All methods
											   const float DeltaSeconds,       // All methods
											   const float DampTime,           // All methods
											   FVector& Output,                // All methods
											   const FVector TemporalInput,    // Spring
											   const FVector SpatialInput,     // Spring
											   const FVector CurrentVelocity,  // ExactSpring
										       const FVector TargetVector,     // ExactSpring
											   const FVector TargetVelocity,   // ExactSpring
											   FVector& OutVelocity,           // ExactSpring
											   const FVector PreviousResidual, // ContinuousNaive
											   FVector& DeltaResidual          // LowPass
											);

	/** Collectively damp a rotator using different damp methods with the same damp time.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input rotator to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output rotator.
	 * @param (Spring) TemporalInput - Camera `current position` - `previous position`
	 * @param (Spring) SpatialInput - Camera `desired position` - `current position`
	 * @param (ExactSpring) CurrentVelocity - Input vector representing current velocity.
	 * @param (ExactSpring) TargetVector - Input vector representing target property, e.g., position.
	 * @param (ExactSpring) TargetVelocity - Input vector representing target velocity.
	 * @param (ExactSpring) OutVelocity - Ouput velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampRotatorWithSameDampTime(const FDampParams DampParams,   // All methods
											    const FRotator Input,           // All methods
											    const float DeltaSeconds,       // All methods
											    const float DampTime,           // All methods
											    FRotator& Output,               // All methods
											    const FVector TemporalInput,    // Spring
											    const FVector SpatialInput,     // Spring
											    const FVector CurrentVelocity,  // ExactSpring
											    const FVector TargetVector,     // ExactSpring
											    const FVector TargetVelocity,   // ExactSpring
											    FVector& OutVelocity,           // ExactSpring
											    const FVector PreviousResidual, // ContinuousNaive
												FVector& DeltaResidual          // LowPass
										);

	/** Collectively damp a vector using different damp methods with different damp times.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input vector to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output vector.
	 * @param (Spring) TemporalInput - Camera `current position` - `previous position`
	 * @param (Spring) SpatialInput - Camera `desired position` - `current position`
	 * @param (ExactSpring) CurrentVelocity - Input vector representing current velocity.
	 * @param (ExactSpring) TargetVector - Input vector representing target property, e.g., position.
	 * @param (ExactSpring) TargetVelocity - Input vector representing target velocity.
	 * @param (ExactSpring) OutVelocity - Ouput velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampVectorWithDifferentDampTime(const FDampParams DampParams,   // All methods
											  	    const FVector Input,            // All methods
												    const float DeltaSeconds,       // All methods
												    const FVector DampTime,         // All methods
												    FVector& Output,                // All methods
												    const FVector TemporalInput,    // Spring
												    const FVector SpatialInput,     // Spring
												    const FVector CurrentVelocity,  // ExactSpring
												    const FVector TargetVector,     // ExactSpring
												    const FVector TargetVelocity,   // ExactSpring
												    FVector& OutVelocity,           // ExactSpring
												    const FVector PreviousResidual, // ContinuousNaive
													FVector& DeltaResidual          // LowPass
											);


	/** Collectively damp a rotator using different damp methods with different damp times.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input rotator to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output rotator.
	 * @param (Spring) TemporalInput - Camera `current position` - `previous position`
	 * @param (Spring) SpatialInput - Camera `desired position` - `current position`
	 * @param (ExactSpring) CurrentVelocity - Input vector representing current velocity.
	 * @param (ExactSpring) TargetVector - Input vector representing target property, e.g., position.
	 * @param (ExactSpring) TargetVelocity - Input vector representing target velocity.
	 * @param (ExactSpring) OutVelocity - Ouput velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampRotatorWithDifferentDampTime(const FDampParams DampParams,   // All methods
												     const FRotator Input,           // All methods
												     const float DeltaSeconds,       // All methods
												     const FVector DampTime,         // All methods
												     FRotator& Output,               // All methods
												     const FVector TemporalInput,    // Spring
												     const FVector SpatialInput,     // Spring
												     const FVector CurrentVelocity,  // ExactSpring
												     const FVector TargetVector,     // ExactSpring
												     const FVector TargetVelocity,   // ExactSpring
												     FVector& OutVelocity,           // ExactSpring
												     const FVector PreviousResidual, // ContinuousNaive
													 FVector& DeltaResidual          // LowPass
											);

	/** Damp a value using different damp methods.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input value to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output value.
	 * @param (Spring) TemporalInput - Camera `current position` - `previous position`
	 * @param (Spring) SpatialInput - Camera `desired position` - `current position`
	 * @param (Spring) SpringCoefficient - Controls how responsive camera gets back to its rest place. The larger, the more responsive.
	 * @param (ExactSpring) CurrentVelocity - Input value representing current velocity.
	 * @param (ExactSpring) TargetValue - Input value representing target property, e.g., position.
	 * @param (ExactSpring) TargetVelocity - Input value representing target velocity.
	 * @param (ExactSpring) DampRatio - Damp ratio used for the input value. A value of 1 means a critically damped spring, a value <1 means an under-damped spring, and a value of >1 means a over-damped spring.
	 * @param (ExactSpring) HalfLife - Duration of time used to damp the input value. Cannot be negative.
	 * @param (ExactSpring) OutVelocity - Ouput velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDamp(const FDampParams DampParams,   // All methods
						 const float Input,              // All methods
						 const float DeltaSeconds,       // All methods
						 const float DampTime,           // All methods
						 float& Output,                  // All methods
						 const float TemporalInput,      // Spring
		                 const float SpatialInput,       // Spring
						 const float SpringCoefficient,  // Spring
						 const float CurrentVelocity,    // ExactSpring
						 const float TargetValue,        // ExactSpring
						 const float TargetVelocity,     // ExactSpring
						 const float DampRatio,          // ExactSpring
						 const float HalfLife,           // ExactSpring
						 float& OutVelocity,             // ExactSpring
						 const float PreviousResidual,   // ContinuousNaive
					     float& DeltaResidual          // LowPass
				);

	/** Damp a float with a given damp time. 
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input float to damp.
	 * @param DampTime - Damp time.
	 * @param Output - Resulting damped output float.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampSingleValue"))
	static void DamperValue(const FDampParams& DampParams, const float& DeltaSeconds, const float& Input, float DampTime, float& Output);

	/** Damp a vector with the same damp time for all elements in the vector.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input vector to damp.
	 * @param DampTime - Damp time (float).
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampVectorWithSameDampTime"))
	static void DamperVectorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, float DampTime, FVector& Output);

	/** Damp a vector with different damp times for different elements in the vector.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input vector to damp.
	 * @param DampTime - Damp time (vector).
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampVectorWithDifferentDampTimes"))
	static void DamperVectorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, FVector DampTime, FVector& Output);

	/** Damp a rotator with the same damp time for all elements in the vector.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input rotator to damp.
	 * @param DampTime - Damp time (float).
	 * @param Output - Resulting damped output rotator.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampRotatorWithSameDampTime"))
	static void DamperRotatorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, float DampTime, FRotator& Output);

	/** Damp a rotator with different damp times for different elements in the rotator.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input rotator to damp.
	 * @param DampTime - Damp time (vector).
	 * @param Output - Resulting damped output rotator.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampRotatorWithDifferentDampTimes"))
	static void DamperRotatorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, FVector DampTime, FRotator& Output);

	/** Use a simple spring mass system to damp a vector. Note that spring damping may cause unstable camera behavior.
	 *  If it is not what you really want, you are supposed to use other damping methods. 
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param TemporalInput - Camera `current position` - `previous position`
	 * @param SpatialInput - Camera `desired position` - `current position` 
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "SpringDampVector"))
	static void SpringDampVector(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& TemporalInput, const FVector& SpatialInput, FVector& Output);

	/** Use a simple spring mass system to damp a value. Note that spring damping may cause unstable camera behavior.
	 *  If it is not what you really want, you are supposed to use other damping methods.
	 * @param MaxDeltaSeconds - Max delta seconds, default is 1 / 60.0f.
	 * @param SpringCoefficient - Controls how responsive camera gets back to its rest place. The larger, the more responsive.
	 * @param SpringResidual - Damp residual after damp time. Small values mean more responsiveness.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param TemporalInput - Camera `current position` - `previous position`
	 * @param SpatialInput - Camera `desired position` - `current position`
	 * @param Output - Resulting damped output value.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "SpringDampValue"))
	static void SpringDampValue(const float& MaxDeltaSeconds, const float& SpringCoefficient, const float& SpringResidual, const float& DeltaSeconds, const float& TemporalInput, const float& SpatialInput, float& Output);

	/** An exact spring damper used to damp an input vector. Credit to: https://theorangeduck.com/page/spring-roll-call#dampingratio. 
	 * @param CurrentVector - Input vector representing current property, e.g., position.
	 * @param CurrentVelocity - Input vector representing current velocity.
	 * @param TargetVector - Input vector representing target property, e.g., position.
	 * @param TargetVelocity - Input vector representing target velocity.
	 * @param DampRatio - Damp ratio used for the input vector. A value of 1 means a critically damped spring, a value <1 means an under-damped spring, and a value of >1 means a over-damped spring.
	 * @param HalfLife - Duration of time used to damp the input vector. Cannot be negative.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param OutVector - Output vector representing current property, e.g., position.
	 * @param OutVelocity - Ouput velocity.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "ExactSpringDampVector"))
	static void ExactSpringDamperVector(const FVector& CurrentVector, const FVector& CurrentVelocity, const FVector& TargetVector, const FVector& TargetVelocity, FVector DampRatio, FVector HalfLife, const float& DeltaSeconds, FVector& OutVector, FVector& OutVelocity);

	/** An exact spring damper used to damp an input value. Credit to: https://theorangeduck.com/page/spring-roll-call#dampingratio.
	 * @param CurrentValue - Input value representing current property, e.g., position.
	 * @param CurrentVelocity - Input value representing current velocity.
	 * @param TargetValue - Input value representing target property, e.g., position.
	 * @param TargetVelocity - Input value representing target velocity.
	 * @param DampRatio - Damp ratio used for the input vector. A value of 1 means a critically damped spring, a value <1 means an under-damped spring, and a value of >1 means a over-damped spring. Cannot be negative.
	 * @param HalfLife - Duration of time used to damp the input value. Cannot be negative.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param OutValue - Output value representing current property, e.g., position.
	 * @param OutVelocity - Output velocity.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "ExactSpringDampValue"))
	static void ExactSpringDamperValue(const float& CurrentValue, const float& CurrentVelocity, const float& TargetValue, const float& TargetVelocity, float DampRatio, float HalfLife, const float& DeltaSeconds, float& OutValue, float& OutVelocity);

	/** Damp a vector with the same damp time for all elements in the vector. This is an optimized algorithm with continuous fourier series approximation.
	 * @param DampParams - A set of damp parameters.
	 * @param Input - Input vector to damp.
	 * @param PreviousResidual - Camera residuals at the previous frame.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Damp time (float). Will override DampParams.DampTime.
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampVectorWithSameDampTime (Continuity Optimization)"))
	static void ContinuousDamperVectorWithSameDampTime(const FDampParams& DampParams, const FVector& Input, const FVector& PreviousResidual, const float& DeltaSeconds, float DampTime, FVector& Output);

	/** Damp a vector with different damp times for all elements in the vector. This is an optimized algorithm with continuous fourier series approximation.
	 * @param DampParams - A set of damp parameters.
	 * @param Input - Input vector to damp.
	 * @param PreviousResidual - Camera residuals at the previous frame.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Damp time (vector). Will override DampParams.DampTime.
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampVectorWithDifferentDampTime (Continuity Optimization)"))
	static void ContinuousDamperVectorWithDifferentDampTime(const FDampParams& DampParams, const FVector& Input, const FVector& PreviousResidual, const float& DeltaSeconds, FVector DampTime, FVector& Output);

	/** Damp a value an optimized algorithm via continuous fourier series approximation.
	 * @param DampParams - A set of damp parameters.
	 * @param Input - Input value to damp.
	 * @param PreviousResidual - Camera residual at the previous frame.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Damp time (float). Will override DampParams.DampTime.
	 * @param Output - Resulting damped output value.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampValue (Continuity Optimization)"))
	static void ContinuousDamperValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, float& Output);

	/** Damp a value with restriction optimization. 
	 * @param DampParams - A set of damp parameters.
	 * @param Input - Input value to damp.
	 * @param PreviousResidual - Camera residual at the previous frame.
	 * @param Power - Used for SoftRestrictedNaive. Controls how aggresively to compact the curve.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Damp time (float). Will override DampParams.DampTime.
	 * @param Output - Resulting damped output value.
	 */
	UFUNCTION()
	static void RestrictionDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const int& Power, const float& DeltaSeconds, float DampTime, float& Output);
	
	/** Damp a value with restriction optimization. 
	 * @param DampParams - A set of damp parameters.
	 * @param Input - Input value to damp.
	 * @param PreviousResidual - Camera residual at the previous frame.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Damp time (float). Will override DampParams.DampTime.
	 * @param DeltaResidual - Residual difference between this frame and previous frame.
	 * @param Output - Resulting damped output value.
	 */
	UFUNCTION()
	static void LowpassDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, float& DeltaResidual, float& Output);


	/** A fast version to compute atan. Credit to: https://theorangeduck.com/page/spring-roll-call#dampingratio. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "FastAtan"))
	static float FastAtan(float x);

	/** Interpolate two quaternions according to damp time.
	 * @param Quat1 - Starting quaternion.
	 * @param Quat2 - Destination quaternion.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param DampTime - Time used to interpolate from Quat1 to Quat2.
	 * @param Output - Damped output quaternion.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampQuaternion"))
	static void DamperQuaternion(const FQuat& Quat1, const FQuat& Quat2, const float& DeltaSeconds, float DampTime, FQuat& Output);

	/** Return the camera local space coordinate of an input world space position.
	 * @param Camera - Camera.
	 * @param InputPosition - World space input position.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetCameraLocalSpaceCoordinateWithActor"))
	static FVector GetLocalSpacePosition(const AActor* Camera, const FVector& InputPosition);

	/** Return the camera local space coordinate of an input world space position.
	 * @param PivotPosition - Pivot position, for example, the camera's location.
	 * @param ForwardVector - Reference forward vector, for example, the camera's forward vector.
	 * @param RightVector - Reference right vector, for example, the camera's right vector.
	 * @param UpVector - Reference up vector, for example, the camera's up vector.
	 * @param InputPosition - World space input position.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetCameraLocalSpaceCoordinateWithVectors"))
	static FVector GetLocalSpacePositionWithVectors(const FVector& PivotPosition, const FVector& ForwardVector, const FVector& RightVector, const FVector& UpVector, const FVector& InputPosition);

	/** Call a new camera. Internal use only. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (BlueprintInternalUseOnly = "True", WorldContext = "WorldContextObject", DeterminesOutputType = "CameraClass"))
	static AECameraBase* InternalCallCamera(const UObject* WorldContextObject, TSubclassOf<AECameraBase> CameraClass, FVector SpawnLocation, FRotator SpawnRotation, AActor* FollowTarget, AActor* AimTarget, FName FollowSocket, FName AimSocket, USceneComponent* FollowSceneComponent, USceneComponent* AimSceneComponent, float BlendTime, enum EViewTargetBlendFunction BlendFunc, float BlendExp, bool bLockOutgoing, bool bIsTransitory = false, float LifeTime = 0.0f, bool bPreserveState = false, bool bCheckClass = true, AActor* ParentCamera = nullptr);

	/** Call a TSubclassOf<ECameraBase> class type camera actor. 
	 *  Highly recommending using this node rather than UE's vanilla SetViewTargetWithBlend node.
	 *  If you prefer feeding scene components to which the camera will mount than sockets, use the CallCameraWithSceneComponent node.
	 * @param CameraClass - The camera class type to instantiate.
	 * @param SpawnLocation - The location where the camera should be spawned. Will be (0,0,0) if not set.
	 * @param SpawnRotation - The rotation the camera should be initialized with. Will be (0,0,0) if not set.
	 * @param FollowTarget - The target actor passed into the follow component. Can be null if this camera does not have a follow component.
	 * @param AimTarget - The target actor passed into the aim component. Can be null if this camera does not have an aim component.
	 * @param FollowSocket - Optional socket name. If specified, will use this socket's transform instead of the follow target's. Should be careful of the socket's rotation.
	 * @param AimSocket - Optional socket name. If specified, will use this socket's transform instead of the aim target's. Should be careful of the socket's rotation.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 * @param bIsTransitory - Whether the called camera is transitory. If true, it will be automatically terminated after LifeTime seconds.
	 * @param LifeTime - The life time of the called camera if it is transitory.
	 * @param bPreserveState - Whether the incoming camera tries to preserve outgoing camera's location and rotation. If you specified SpawnLocation and SpawnRotation, you should switch this off.
	 * @param bCheckClass - When enabled, return the current active one if the input CameraClass is the same as the current active camera. When disabled, always create a new camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallCamera", WorldContext = "WorldContextObject", DeterminesOutputType = "CameraClass", AdvancedDisplay = 6, HidePin = "ParentCamera"))
	static AECameraBase* CallCamera(const UObject* WorldContextObject, TSubclassOf<AECameraBase> CameraClass, FVector SpawnLocation, FRotator SpawnRotation, AActor* FollowTarget, AActor* AimTarget, FName FollowSocket, FName AimSocket, float BlendTime, enum EViewTargetBlendFunction BlendFunc, float BlendExp, bool bLockOutgoing, bool bIsTransitory = false, float LifeTime = 0.0f, bool bPreserveState = false, bool bCheckClass = true, AActor* ParentCamera = nullptr);

	/** Call a TSubclassOf<ECameraBase> class type camera actor. 
	 *  Highly recommending using this node rather than UE's vanilla SetViewTargetWithBlend node.
	 *  This version should only be used when you prefer feeding scene components rather than sockets. Otherwise, CallCamera node is recommended.
	 * @param CameraClass - The camera class type to instantiate.
	 * @param SpawnLocation - The location where the camera should be spawned. Will be (0,0,0) if not set.
	 * @param SpawnRotation - The rotation the camera should be initialized with. Will be (0,0,0) if not set.
	 * @param FollowTarget - The target actor passed into the follow component. Can be null if this camera does not have a follow component.
	 * @param AimTarget - The target actor passed into the aim component. Can be null if this camera does not have an aim component.
	 * @param FollowSceneComponent - Optional scene component. If specified, will use this component's transform instead of the follow target's. It's your duty to ensure it's valid and paired with FollowTarget.
	 * @param AimSceneComponent - Optional scene component. If specified, will use this component's transform instead of the aim target's. It's your duty to ensure it's valid and paired with AimTarget.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 * @param bIsTransitory - Whether the called camera is transitory. If true, it will be automatically terminated after LifeTime seconds.
	 * @param LifeTime - The life time of the called camera if it is transitory.
	 * @param bPreserveState - Whether the incoming camera tries to preserve outgoing camera's location and rotation. If you specified SpawnLocation and SpawnRotation, you should switch this off.
	 * @param bCheckClass - When enabled, return the current active one if the input CameraClass is the same as the current active camera. When disabled, always create a new camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallCameraWithSceneComponent", WorldContext = "WorldContextObject", DeterminesOutputType = "CameraClass", AdvancedDisplay = 6, HidePin = "ParentCamera"))
	static AECameraBase* CallCameraWithSceneComponent(const UObject* WorldContextObject, TSubclassOf<AECameraBase> CameraClass, FVector SpawnLocation, FRotator SpawnRotation, AActor* FollowTarget, AActor* AimTarget, USceneComponent* FollowSceneComponent, USceneComponent* AimSceneComponent, float BlendTime, enum EViewTargetBlendFunction BlendFunc, float BlendExp, bool bLockOutgoing, bool bIsTransitory = false, float LifeTime = 0.0f, bool bPreserveState = false, bool bCheckClass = true, AActor* ParentCamera = nullptr);

	/** Call an animated camera, i.e., driven by an animation sequence. This is usually used inside a skill.
	 * @param AnimToPlay - The animation sequence you want to play on camera.
	 * @param RefCoordinate - In which reference frame you want to play the camera animation.
	 * @param RefCoordinateActor - In which actor's local space you want to play the camera animation.
	 * @param PositionOffset - Position offset, in reference space.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallAnimatedCamera", WorldContext = "WorldContextObject"))
	static AECameraBase* CallAnimatedCamera(const UObject* WorldContextObject, UAnimSequence* AnimToPlay, FTransform RefCoordinate, AActor* RefCoordinateActor = nullptr, FVector PositionOffset = FVector(0, 0, 0), float BlendTime = 0.0f, enum EViewTargetBlendFunction BlendFunc = EViewTargetBlendFunction::VTBlend_Linear, float BlendExp = 2.0f, bool bLockOutgoing = false);
	
	/** Call a sequenced camera.
	 * @param SequencedCameraClass - A class of type ESequencedCameraSetupActor specifying sub-camera with their blending setups.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallSequencedCamera", WorldContext = "WorldContextObject", DeterminesOutputType = "SequencedCameraClass", AdvancedDisplay = 6))
	static AESequencedCameraSetupActor* CallSequencedCamera(const UObject* WorldContextObject, TSubclassOf<AESequencedCameraSetupActor> SequencedCameraClass);

	/** Immediately terminate current active camera and gets back to previous active camera.
	 *  If you only want to call a new camera, use CallCamera node instead, NOT this node.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "TerminateActiveCamera", WorldContext = "WorldContextObject"))
	static void TerminateActiveCamera(const UObject* WorldContextObject);

	/** Get the current active `ECameraBase` camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetActiveCamera", WorldContext = "WorldContextObject"))
	static AECameraBase* GetActiveCamera(const UObject* WorldContextObject);

	/** Get EPlayerCameraManager. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetEPlayerCameraManager", WorldContext = "WorldContextObject"))
	static AEPlayerCameraManager* GetEPlayerCameraManager(const UObject* WorldContextObject, int32 index);

	/** Get the position applied with local space offset. Input is actor.
	 * @param TargetActor - An input actor in whose local space offset is applied.
	 * @param Offset - Offset applied to the actor's local space.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetPositionWithLocalOffset(Actor)"))
	static FVector GetPositionWithLocalOffset(AActor* TargetActor, const FVector& Offset);

	/** Get the position applied with local space offset. Input is position and rotation.
	 * @param Position - Target position.
	 * @param Rotation - Target rotation.
	 * @param Offset - Offset applied to the actor's local space.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "GetPositionWithLocalOffset(Rotation)"))
	static FVector GetPositionWithLocalRotatedOffset(const FVector& Position, const FRotator& Rotation, const FVector& Offset);

	/** Applies a post process material to the player camera manager. You should use EPlayerCameraManager to make this node work.
	 *  @param InBlendableObject - The post process material to add.
	 *  @param InWeight - Amount of influence the post process effect will have. 1 means full effect, while 0 means no effect.
	 * 	@param InBlendInTime - Time used to blend into this post process.
	 *	@param InDuration - Duration for this post process. Set as 0 to keep it infinite.
	 *  @param InBlendOutTime - Time used to blend out of this post process.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "AddBlendable", WorldContext = "WorldContextObject"))
	static void AddBlendable(const UObject* WorldContextObject, const TScriptInterface<IBlendableInterface>& InBlendableObject, const float InWeight = 1.f, const float InBlendInTime = 0.5f, const float InDuration = 0.0f, const float InBlendOutTime = 0.5f);

	/** Removes a post process material added to the player camera manager. You should use EPlayerCameraManager to make this node work.
	 *  @param InBlendableObject - The post process material to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "RemoveBlendable", WorldContext = "WorldContextObject"))
	static void RemoveBlendable(const UObject* WorldContextObject, const TScriptInterface<IBlendableInterface>& InBlendableObject);


	/** Start camera shake with shake class and optional packed oscillation parameters.
	 * @param ShakeClass - The class of camera shake to play.
	 * @param ShakeParams - Packed oscillation parameters. If ShakeClass is not specified, this parameter will be used.
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStartCameraShake", WorldContext = "WorldContextObject", AdvancedDisplay = 3))
	static UCameraShakeBase* EasyStartCameraShake(const UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, FPackedOscillationParams ShakeParams, float Scale, ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot, bool bSingleInstance);
	
	/** Start camera shake from source with shake class and optional packed oscillation parameters. This function is intended for temoprary camera shake, 
	 * i.e., camera shakes with finite duration. If you are using infinite camera shakes, though you can still specify an inifite time
	 * in this node, it's highly recommended to use UE's built-in `Start Camera Shake From Source` nodes and `Stop Camera Shake` to 
	 * terminate specific camera shake instances.
	 * @param ShakeClass - The class of camera shake to play.
	 * @param ShakeParams - Packed oscillation parameters. If ShakeClass is not specified, this parameter will be used.
	 * @param SpawnActor - Actor to which the camera shake source actor will be spawned and attached.
	 * @param SpawnLocation - Used when SpawnActor is null. Location where the camera shake source actor will be spawned.
	 * @param Attenuation - The attenuation profile for how camera shakes' intensity falls off with distance.
	 * @param InnerAttenuationRadius - Under this distance from the source, the camera shakes are at full intensity.
	 * @param OuterAttenuationRadius - Outside of this distance from the source, the cmaera shakes don't apply at all.
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStartCameraShakeFromSource", WorldContext = "WorldContextObject", AdvancedDisplay = 5))
	static UCameraShakeBase* EasyStartCameraShakeFromSource(const UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, FPackedOscillationParams ShakeParams, AActor* SpawnActor, FVector SpawnLocation, ECameraShakeAttenuation Attenuation, float InnerAttenuationRadius, float OuterAttenuationRadius, float Scale, ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot, bool bSingleInstance);

	/**
	 * Does a camera fade to a solid color and then fades back.  Animates automatically.
	 * @param FromAlpha - Alpha at which to begin the fade. Range [0..1], where 0 is fully transparent and 1 is fully opaque solid color.
	 * @param ToAlpha - Alpha at which to finish in-fade.
	 * @param FadeInTime - How long the in-fade should take, in seconds.
	 * @param FadeInFunc - Blend function for in-fade.
	 * @param Duration - How long ToAlpha remains after finishing in-fase, in seconds.
	 * @param FadeOutTime - How long the out-fade should take, in seconds.
	 * @param FadeOutFunc - Blend function for out-fade.
	 * @param Color - Color to fade to/from.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStartCameraFade", WorldContext = "WorldContextObject", AdvancedDisplay = 3))
	static void EasyStartCameraFade(const UObject* WorldContextObject, float FromAlpha, float ToAlpha, float FadeInTime, TEnumAsByte<EEasingFunc::Type> FadeInFunc, float Duration, float FadeOutTime, TEnumAsByte<EEasingFunc::Type> FadeOutFunc, FLinearColor Color);

	/**
	 * Stops camera fading.
	 * @param StopAlpha - Alpha at which fade stops.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStopCameraFade", WorldContext = "WorldContextObject"))
	static void EasyStopCameraFade(const UObject* WorldContextObject, float StopAlpha = 0.0f);
	
	/**
	 * Custom thunk function used to get and set a property's value. Does not support float and double properties.
	 * @param Object - The object that owns this property.
	 * @param PropertyName - Name of property you want to get value from or set to. Note this should be the full name rather than the display name. For most boolean values, a prefix `b` should be appended.
	 * @param Value - The value you want to set to or get from the object's property.
	 * @param Setter - If true, this function acts as a setter, otherwise a getter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, CustomThunk, Category = "ECamera|Utils", meta = (CustomStructureParam = "Value", DisplayName = "AccessCameraPropertyByName"))
	static void AccessCameraPropertyByName(UObject* Object, FName PropertyName, const int32& Value, bool bSetter);
	DECLARE_FUNCTION(execAccessCameraPropertyByName)
	{
		P_GET_OBJECT(UObject, OwnerObject);
		P_GET_PROPERTY(FNameProperty, PropertyName);

		Stack.StepCompiledIn<FStructProperty>(nullptr);
		void* ValuePtr = Stack.MostRecentPropertyAddress;
		FProperty* ValueProp = Stack.MostRecentProperty;

		P_GET_UBOOL(bSetter);

		P_FINISH;

		P_NATIVE_BEGIN;
		Generic_AccessCameraPropertyByName(OwnerObject, PropertyName, ValuePtr, ValueProp, bSetter);
		P_NATIVE_END;
	}


public:
	static void Generic_AccessCameraPropertyByName(UObject* OwnerObject, FName PropertyName, void* ValuePtr, FProperty* ValueProp, bool bSetter);
};
