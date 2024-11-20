// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include "ECameraTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "ESetPropertyLatentAction.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "ECameraLibrary.generated.h"

class AEPlayerCameraManager;

/**
 * UCameraLibrary contains functions for blueprint callable nodes.
 */
UCLASS()
class COMPONENTCAMERASYSTEM_API UECameraLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static std::vector<float> Factorials;

public:
	/** Collectively damp a vector using different damp methods with the same damp time.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input vector to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output vector.
	 * @param (Spring) CurrentVelocity - Current frame's velocity.
	 * @param (Spring) OutVelocity - Next frame's velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampVectorWithSameDampTime(const FDampParams DampParams,   // All methods
											   const FVector Input,            // All methods
											   const float DeltaSeconds,       // All methods
											   const float DampTime,           // All methods
											   FVector& Output,                // All methods
											   const FVector CurrentVelocity,  // Spring
											   FVector& OutVelocity,           // Spring
											   const FVector PreviousResidual, // ContinuousNaive
											   FVector& DeltaResidual          // LowPass
											);

	/** Collectively damp a rotator using different damp methods with the same damp time.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input rotator to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output rotator.
	 * @param (Spring) CurrentVelocity - Current frame's velocity.
	 * @param (Spring) OutVelocity - Next frame's velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampRotatorWithSameDampTime(const FDampParams DampParams,   // All methods
											    const FRotator Input,           // All methods
											    const float DeltaSeconds,       // All methods
											    const float DampTime,           // All methods
											    FRotator& Output,               // All methods
												const FVector CurrentVelocity,  // Spring
												FVector& OutVelocity,           // Spring
											    const FVector PreviousResidual, // ContinuousNaive
												FVector& DeltaResidual          // LowPass
										);

	/** Collectively damp a vector using different damp methods with different damp times.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input vector to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output vector.
	 * @param (Spring) CurrentVelocity - Current frame's velocity.
	 * @param (Spring) OutVelocity - Next frame's velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampVectorWithDifferentDampTime(const FDampParams DampParams,   // All methods
											  	    const FVector Input,            // All methods
												    const float DeltaSeconds,       // All methods
												    const FVector DampTime,         // All methods
												    FVector& Output,                // All methods
													const FVector CurrentVelocity,  // Spring
													FVector& OutVelocity,           // Spring
												    const FVector PreviousResidual, // ContinuousNaive
													FVector& DeltaResidual          // LowPass
											);


	/** Collectively damp a rotator using different damp methods with different damp times.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input rotator to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output rotator.
	 * @param (Spring) CurrentVelocity - Current frame's velocity.
	 * @param (Spring) OutVelocity - Next frame's velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDampRotatorWithDifferentDampTime(const FDampParams DampParams,   // All methods
												     const FRotator Input,           // All methods
												     const float DeltaSeconds,       // All methods
												     const FVector DampTime,         // All methods
												     FRotator& Output,               // All methods
													 const FVector CurrentVelocity,  // Spring
													 FVector& OutVelocity,           // Spring
												     const FVector PreviousResidual, // ContinuousNaive
													 FVector& DeltaResidual          // LowPass
											);

	/** Damp a value using different damp methods.
	 * @param (All) DampParams - A set of damp parameters.
	 * @param (All) Input - Input value to damp.
	 * @param (All) DeltaSeconds - The elapsed time since last frame.
	 * @param (All) DampTime - Damp time. Will override DampParams.DampTime.
	 * @param (All) Output - Resulting damped output value.
	 * @param (Spring) Frequency - Controls the frequency of oscillation and the speed of decay.
	 * @param (Spring) DampRatio - Controls whether the spring is undamped (=0), underdamped (<1), critically damped (=1), or overdamped (>1).
	 * @param (Spring) CurrentVelocity - Current frame's velocity.
	 * @param (Spring) OutVelocity - Next frame's velocity.
	 * @param (ContinuousNaive) PreviousResidual - Camera residuals at the previous frame.
	 * @param (LowPass) DeltaResidual - Residual difference between this frame and previous frame.
	 */
	UFUNCTION()
	static void EasyDamp(const FDampParams DampParams,   // All methods
						 const float Input,              // All methods
						 const float DeltaSeconds,       // All methods
						 const float DampTime,           // All methods
						 double& Output,                 // All methods
						 const float Frequency,          // Spring
		                 const float DampRatio,          // Spring
						 const float CurrentVelocity,    // Spring
						 double& OutVelocity,            // Spring
						 const float PreviousResidual,   // ContinuousNaive
						 double& DeltaResidual           // LowPass
				);

	/** Damp a float with a given damp time. 
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input float to damp.
	 * @param DampTime - Damp time.
	 * @param Output - Resulting damped output float.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampSingleValue"))
	static void DamperValue(const FDampParams& DampParams, const float& DeltaSeconds, const float& Input, float DampTime, double& Output);

	/** Damp a float with a given damp time using the naive damp algorithm.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input float to damp.
	 * @param Output - Resulting damped output float.
	 * @param DampTime - Damp time (float).
	 * @param Residual - Damp residual after damp time (in percent).
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "NaiveDampSingleValue"))
	static void NaiveDamperValue(const float& DeltaSeconds, const float& Input, double& Output, float DampTime = 0.2f, float Residual = 0.01f);

	/** Damp a vector with the same damp time for all elements in the vector.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input vector to damp.
	 * @param DampTime - Damp time (float).
	 * @param Output - Resulting damped output vector.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampVectorWithSameDampTime"))
	static void DamperVectorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, float DampTime, FVector& Output);

	/** Damp a vector with the same damp time for all elements in the vector using the naive damp algorithm.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input vector to damp.
	 * @param Output - Resulting damped output vector.
	 * @param DampTime - Damp time (float).
	 * @param Residual - Damp residual after damp time (in percent).
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "NaiveDampVectorWithSameDampTime"))
	static void NaiveDamperVectorWithSameDampTime(const float& DeltaSeconds, const FVector& Input, FVector& Output, float DampTime = 0.2f, FVector Residual = FVector(0.01f));

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

	/** Damp a rotator with the same damp time for all elements in the vector using the naive damp algorithm.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input rotator to damp.
	 * @param Output - Resulting damped output rotator.
	 * @param DampTime - Damp time (float). Will override DampParams.DampTime.
	 * @param Residual - Damp residual after damp time (in percent).
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "NaiveDampRotatorWithSameDampTime"))
	static void NaiveDamperRotatorWithSameDampTime(const float& DeltaSeconds, const FRotator& Input, FRotator& Output, float DampTime = 0.2f, FVector Residual = FVector(0.01f));

	/** Damp a rotator with different damp times for different elements in the rotator.
	 * @param DampParams - A set of damp parameters.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input rotator to damp.
	 * @param DampTime - Damp time (vector).
	 * @param Output - Resulting damped output rotator.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "DampRotatorWithDifferentDampTimes"))
	static void DamperRotatorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, FVector DampTime, FRotator& Output);

	/** Use a simple spring mass system to damp a value. Note that spring damping may cause unstable camera behavior.
	 *  If it is not what you really want, you are supposed to use other damping methods.
	 * @param Frequency - Controls the frequency of oscillation and the speed of decay.
	 * @param DampRatio - Controls whether the spring is undamped (=0), underdamped (<1), critically damped (=1), or overdamped (>1).
	 * @param CurrentVelocity - Current frame's velocity.
	 * @param OutVelocity - Next frame's velocity.
	 * @param DeltaSeconds - The elapsed time since last frame.
	 * @param Input - Input value to damp.
	 * @param Output - Resulting damped output value.
	 */
	UFUNCTION(BlueprintPure, Category = "ECamera|Utils", meta = (DisplayName = "SpringDampValue"))
	static void SpringDampValue(const float& Frequency, const float& DampRatio, const float& CurrentVelocity, double& OutVelocity, const float& DeltaSeconds, const float& Input, double& Output);

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
	static void ContinuousDamperValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, double& Output);

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
	static void RestrictionDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const int& Power, const float& DeltaSeconds, float DampTime, double& Output);
	
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
	static void LowpassDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, double& DeltaResidual, double& Output);


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
	 * @param CoordinateActor - In which actor's local space you want to play the camera animation.
	 * @param Coordinate - In which reference frame you want to play the camera animation.
	 * @param PositionOffset - Position offset, in reference space.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallAnimatedCamera", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Coordinate", AdvancedDisplay = 3))
	static AECameraBase* CallAnimatedCamera(const UObject* WorldContextObject, UAnimSequence* AnimToPlay, AActor* CoordinateActor = nullptr, const FTransform& Coordinate = FTransform(), FVector PositionOffset = FVector(0, 0, 0), float BlendTime = 0.0f, enum EViewTargetBlendFunction BlendFunc = EViewTargetBlendFunction::VTBlend_Linear, float BlendExp = 2.0f, bool bLockOutgoing = false);
	
	/** Call a keyframed camera.
	 * @param KeyframedCamera - The keyframed camera you want to play.
	 * @param CoordinateActor - In which actor's local space you want to apply the keyframes.
	 * @param CoordinateSocket - CoordinateActor's socket the camera is based on. If this is specified, this socket's local space will be used. 
	 * @param Coordinate - In which reference frame you want to apply the keyframes.
	 * @param bCoordinateLocationOnly - Whether to apply only keyframed positions in the specified CoordinateActor or Coordinate space.
	 * @param LocationOffset - Position offset, in reference coordinate space.
	 * @param AimOverride - The actor at which the camera will tries to aim.
	 * @param AimSocket - The socket of AimOverride at which the camera will tries to aim. If null or None, it will be the actor's root.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "CallKeyframedCamera", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Coordinate", AdvancedDisplay = 3))
	static AECameraBase* CallKeyframedCamera(const UObject* WorldContextObject, TSubclassOf<class AEKeyframedCamera> KeyframedCamera, AActor* CoordinateActor = nullptr, FName CoordinateSocket = FName("None"), const FTransform & Coordinate = FTransform(), bool bCoordinateLocationOnly = false, FVector LocationOffset = FVector(0, 0, 0), AActor * AimOverride = nullptr, FName AimSocket = FName("None"), float BlendTime = 0.0f, enum EViewTargetBlendFunction BlendFunc = EViewTargetBlendFunction::VTBlend_Linear, float BlendExp = 2.0f, bool bLockOutgoing = false);
	
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

	/** A sugar of ControlAim's StartRecentering function. If current active camera does not have a ControlAim component, this function will do nothing.
	  * 
	  * Manually starts recentering. Used when recentering scheme is Manual, otherwise this function does nothing. 
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
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStartRecentering", WorldContext = "WorldContextObject"))
	static void EasyStartRecentering(const UObject* WorldContextObject, float Duration, TEnumAsByte<EEasingFunc::Type> Func, float Exp);

	/** A sugar of ControlAim's StopRecentering function. If current active camera does not have a ControlAim component, this function will do nothing.
	  * 
	  * Immediately stop recentering. Used when recentering scheme is Manual. This function may be called when the follow target starts to move. 
	  */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Utils", meta = (DisplayName = "EasyStopRecentering", WorldContext = "WorldContextObject"))
	static void EasyStopRecentering(const UObject* WorldContextObject);

	/** Latent function to set a property of a given object. You can type nested property names, e.g., {Property A}.{Property B}.
	  * Note that this node does not support container types array/set/map, and the destination property can either be a numeric type (int/float) or struct, but not an object.
	  * If it's a struct, values in struct will be recursively set, but ignoring any Object type property.
	  * For example, if you want to set the CameraDistance of a ScreenFollow component, you can achieve this using one of the following options:
	  * 1) Object is ECameraBase, PropertyName is CameraSettingsComponent.FollowComponent.CameraDistance.
	  * 2) Object is CameraSettingsComponent, PropertyName is FollowComponent.CameraDistance.
	  * 3) Object is ScreenFollow, PropertyName is CameraDistance.
	  * 
	  * You should make sure the property name is correct and matches the value type. That is, if PropertyName is DampTime, Value should be of a Vector type.
	  * If no such PropertyName exists, input Object is not valid, or Value type does not match the found property, this node will do nothing.
	  * If another SetPropertyByName node intends to set the same property of the same object as this one, or the object becomes invalid, this node will invoke the OnInterrupt output pin.
	  * 
	  * @param Object: The input object.
	  * @param PropertyName: The name of the property to set.
	  * @param Value: New property value.
	  * @param Duration: Blend duration.
	  * @param Func: Blend function.
	  * @param Exp: Blend exponential.
	  */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "ECamera|Utils", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", CustomStructureParam = "Value", ExpandEnumAsExecs = "OutPin", AdvancedDisplay = 4))
	static void SetPropertyByName(const UObject* WorldContextObject, UObject* Object, FName PropertyName, const int32& Value, double Duration, TEnumAsByte<EEasingFunc::Type> Func, double Exp, FLatentActionInfo LatentInfo, ELatentOutputPins& OutPin);
	DECLARE_FUNCTION(execSetPropertyByName)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_OBJECT(UObject, Object);
		P_GET_PROPERTY(FNameProperty, PropertyName);
		Stack.StepCompiledIn<FProperty>(NULL);
		void* ValuePtr = Stack.MostRecentPropertyAddress;
		FProperty* ValueProperty = Stack.MostRecentProperty;
		P_GET_PROPERTY(FDoubleProperty, Duration);
		P_GET_PROPERTY(FByteProperty, Func);
		P_GET_PROPERTY(FDoubleProperty, Exp);
		P_GET_STRUCT(FLatentActionInfo, LatentInfo);
		P_GET_ENUM_REF(ELatentOutputPins, OutPin);
		
		P_FINISH;
		P_NATIVE_BEGIN;
		auto [SrcProperty, SrcPtr] = GetNestedPropertyFromObject(Object, PropertyName);
		if (SrcProperty == nullptr || ValueProperty == nullptr)
		{
			return;
		}

		bool bSameType = SrcProperty->SameType(ValueProperty);
		bool bFloatType = SrcProperty->IsA<FFloatProperty>() && ValueProperty->IsA<FDoubleProperty>();
		if (bSameType || bFloatType)
		{
			if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
			{
				TArray<FESetPropertyLatentAction*>& ActionList = GetActionList<FESetPropertyLatentAction>();
				FESetPropertyLatentAction** ActionPtr = ActionList.FindByPredicate([SrcProperty = SrcProperty, SrcPtr = SrcPtr](FESetPropertyLatentAction* ThisAction) { return ThisAction->IsSameProperty(SrcProperty, SrcPtr); });
				FESetPropertyLatentAction* Action = ActionPtr == nullptr ? nullptr : *ActionPtr;
				if (Action != nullptr)
				{
					Action->SetInterrupt(true);
				}

				Action = new FESetPropertyLatentAction(Object, SrcProperty, SrcPtr, ValueProperty, ValuePtr, Duration, EEasingFunc::Type(Func), Exp, OutPin, LatentInfo);
				Action->OnActionCompletedOrInterrupted.AddLambda([&ActionList, &Action]() { ActionList.Remove(Action); });
				ActionList.Add(Action);
				World->GetLatentActionManager().AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("The found property %s does not has the same type as given property %s, respectively are %s and %s"),
				*SrcProperty->NamePrivate.ToString(), *ValueProperty->NamePrivate.ToString(), *SrcProperty->GetCPPType(), *ValueProperty->GetCPPType());
		}
		
		P_NATIVE_END;
	}

	template<typename ActionType>
	// @TODO: Error in Clang 16
	//	requires std::derived_from<ActionType, class FPendingLatentAction>
	static TArray<ActionType*>& GetActionList()
	{
		static TArray<ActionType*> ActionList {};
		return ActionList;
	}

	static FProperty* GetPropertyFromObject(UObject* Object, FName PropertyName)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Warning, TEXT("Input Object is invalid when calling function GetPropertyFromObject."));
			return nullptr;
		}

		UClass* ObjectClass = Object->GetClass();
		FProperty* Property = FindFProperty<FProperty>(ObjectClass, PropertyName);

		if (Property != nullptr)
		{
			return Property;
		}

#if WITH_EDITORONLY_DATA
		UBlueprint* Blueprint = Cast<UBlueprint>(ObjectClass->ClassGeneratedBy);

		if (Blueprint == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find Property %s in Object %s when calling function GetPropertyFromObject."), *PropertyName.ToString(), *Object->GetName());
			return nullptr;
		}

		Property = FindFProperty<FProperty>(Blueprint->GetClass(), PropertyName);
		if (Property == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find Property %s in Object %s when calling function GetPropertyFromObject."), *PropertyName.ToString(), *Object->GetName());
			return nullptr;
		}

		return Property;
#else
		return nullptr;
#endif
	}

	static std::pair<FProperty*, void*> GetNestedPropertyFromObject(UObject* Object, FName PropertyName)
	{
		if (!IsValid(Object))
		{
			UE_LOG(LogTemp, Warning, TEXT("Input Object is invalid when calling function GetNestedPropertyFromObject."));
			return std::make_pair(nullptr, nullptr);
		}
		
		return GetNestedPropertyFromObjectStruct(Object, Object->GetClass(), PropertyName.ToString());
	}

private:
	static std::pair<FProperty*, void*> GetNestedPropertyFromObjectStruct(void* Object, UStruct* Struct, const FString& PropertyName /*@TODO: Should use FStringView to improve string efficiency. */)
	{
		int FoundIndex;
		FString CurrentProperty;
		FString NextProperty;
		bool bFoundSeparator = PropertyName.FindChar('.', FoundIndex);
		
		if (bFoundSeparator)
		{
			CurrentProperty = PropertyName.Mid(0, FoundIndex);
			NextProperty = PropertyName.Mid(FoundIndex + 1, PropertyName.Len() - FoundIndex - 1);
		}
		else
		{
			CurrentProperty = PropertyName;
		}

		FProperty* Property = FindFProperty<FProperty>(Struct, FName(CurrentProperty));
		if (Property != nullptr)
		{
			void* Value = Property->ContainerPtrToValuePtr<void>(Object);

			if (NextProperty.IsEmpty())
			{
				if (Property->IsA<FNumericProperty>() || Property->IsA<FStructProperty>())
				{
					return std::make_pair(Property, Value);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Terminate property can only be numeric/struct type. Current type is %s."), *Property->GetClass()->GetName());
					return std::make_pair(nullptr, nullptr);
				}
			}
			else
			{
				const FStructProperty* PropAsStruct = CastField<FStructProperty>(Property);
				const FObjectProperty* PropAsObject = CastField<FObjectProperty>(Property);
				const FArrayProperty* PropAsArray = CastField<FArrayProperty>(Property);
				const FSetProperty* PropAsSet = CastField<FSetProperty>(Property);
				const FMapProperty* PropAsMap = CastField<FMapProperty>(Property);

				if (PropAsArray != nullptr || PropAsSet != nullptr || PropAsMap != nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Function GetNestedPropertyFromObjectStruct currently does not support container type."));
				}
				else if (PropAsStruct != nullptr)
				{
					return GetNestedPropertyFromObjectStruct(Value, PropAsStruct->Struct, NextProperty);
				}
				else if (PropAsObject != nullptr)
				{
					// Now Value points to the pointer that points to the real object. Must let it point to the object instead of the pointer. Ref: DiffUtils.cpp
					UObject* PropObject = *((UObject* const*)Value);
					return GetNestedPropertyFromObjectStruct(PropObject, PropObject->GetClass(), NextProperty);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid property: %s. Non-terminal property can only be an object or struct."), *FString(CurrentProperty));
				}

				return std::make_pair(nullptr, nullptr);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find property %s from UStruct %s."), *FString(CurrentProperty), *Struct->GetName());
			return std::make_pair(nullptr, nullptr);
		}
	}
};
