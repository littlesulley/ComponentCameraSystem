// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Actor.h"
#include "Core/ECameraSettingsComponent.h"
#include "ECameraBase.generated.h"

class USceneComponent;
class UCameraComponent;
class APlayerController;

/**
* All cameras should inherit from this class ECameraBase.
*/
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AECameraBase : public ACameraActor
{
	GENERATED_BODY()

public:
	AECameraBase(const FObjectInitializer& ObjectInitializer);

protected:
	/** Default camera settings component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ECamera|CameraBase")
	UECameraSettingsComponent* CameraSettingsComponent;

public:
	/** Default FOV. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	float DefaultFOV;

	/** Default blend time. Used for automatic call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	float DefaultBlendTime;

	/** Default blend function. Used for automatic call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	TEnumAsByte<EViewTargetBlendFunction> DefaultBlendFunc;

	/** Default blend exponential. Used for automatic call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	float DefaultBlendExp;

	/** Default lock outgoing. Used for automatic call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	bool bDefaultLockOutgoing;

	/** Default preserve camera state. Used for automatic call (prior camera is expired). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|CameraBase")
	bool bDefaultPreserveState;
	
	/** Whether this camera is transitory. */
	bool bIsTransitory;
	/** Life time of this camera. */
	float LifeTime;
	/** Whether to preserve camera state when blending. 
	 *  If true, the camera will try to preserve prior camera's location and rotation.
	 */
	bool bPreserveState;
	/** Parent camera of this one. */
	AActor* ParentCamera;

private:
	float ElaspedTimeOnViewTarget;

public:
	/** Get the ECameraSettingsComponent component owned by this camera. */
	UECameraSettingsComponent* GetSettingsComponent() { return CameraSettingsComponent; }

	/** Get FollowComponent owned by this camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase")
	UECameraComponentFollow* GetFollowComponent() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->FollowComponent : nullptr; }

	/** Get AimComponent owned by this camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase")
	UECameraComponentAim* GetAimComponent() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->AimComponent : nullptr; }

	/** Get FollowComponent owned by this camera and cast it to a given follow component class. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase", meta = (DeterminesOutputType = "FollowClass"))
	UECameraComponentFollow* GetFollowComponentOfClass(TSubclassOf<UECameraComponentFollow> FollowClass) { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->GetFollowComponentOfClass(FollowClass) : nullptr; }

	/** Get AimComponent owned by this camera and cast it to a given aim component class. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase", meta = (DeterminesOutputType = "AimClass"))
	UECameraComponentAim* GetAimComponentOfClass(TSubclassOf<UECameraComponentAim> AimClass) { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->GetAimComponentOfClass(AimClass) : nullptr; }

	/** Get the CameraComponent owned by this camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase")
	UCameraComponent* GetCameraComponent() { return Super::GetCameraComponent(); }

	/** Get a particular extension. Return null if no such extension is found. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase", meta = (DeterminesOutputType = "ExtensionClass"))
	UECameraExtensionBase* GetExtensionOfClass(TSubclassOf<UECameraExtensionBase> ExtensionClass) { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->GetExtensionOfClass(ExtensionClass) : nullptr; }

	/** Get follow target of this camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase")
	AActor* GetFollowTarget() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->FollowTarget : nullptr; }

	/** Get aim target of this camera. */
	UFUNCTION(BlueprintPure, Category = "ECamera|CameraBase")
	AActor* GetAimTarget() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->AimTarget : nullptr; }

	/** Set current camera's follow target OR aim target with blend. Can optionally feed socket and scene component. 
	 *  @TODO: Ugly implementation. Should use K2Node in the future.
	 * @param bResetFollowTarget - Use a new follow target. You should feed in a valid follow target below.
	 * @param bResetFollowSocket - Use a new follow socket. You should feed in a valid follow socket below.
	 * @param bResetFollowSceneComponent - Use a new follow scene component. You should feed in a valid follow scene component below.
	 * @param bResetAimTarget - Use a new aim target. You should feed in a valid aim target below.
	 * @param bResetAimSocket - Use a new aim socket. You should feed in a valid aim socket below.
	 * @param bResetAimSceneComponent - Use a new aim scene component. You should feed in a valid aim scene component below.
	 * @param FollowTarget - The new target actor passed into the follow component. 
	 * @param AimTarget - The new target actor passed into the aim component. 
	 * @param FollowSocket - Follow socket name. If specified, will use this socket's transform instead of the follow target's. Should be careful of the socket's rotation.
	 * @param AimSocket - Aim socket name. If specified, will use this socket's transform instead of the aim target's. Should be careful of the socket's rotation.
	 * @param FollowSceneComponent - Follow scene component. If specified, will use this component's transform instead of the follow target's. It's your duty to ensure it's valid and paired with FollowTarget.
	 * @param AimSceneComponent - Follow scene component. If specified, will use this component's transform instead of the aim target's. It's your duty to ensure it's valid and paired with AimTarget.
	 * @param BlendTime - Blend-in time used for transitioning from the current active camera to the new camera.
	 * @param BlendFunc - Which type of blend function to use.
	 * @param BlendExp - Blend exponential.
	 * @param bLockOutgoing - If true, lock outgoing viewtarget to last frame's camera position for the remainder of the blend.
	 * @param bIsTransitory - Whether the called camera is transitory. If true, it will be automatically terminated after LifeTime seconds.
	 * @param LifeTime - The life time of the called camera if it is transitory.
	 * @param bPreserveState - Whether the incoming camera tries to preserve outgoing camera's location and rotation. If you specified SpawnLocation and SpawnRotation, you should switch this off.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase", meta = (AdvancedDisplay = 6))
	AECameraBase* SetFollowAndAimWithBlend(bool bResetFollowTarget, 
										   bool bResetFollowSocket, 
										   bool bResetFollowSceneComponent,
										   bool bResetAimTarget,
										   bool bResetAimSocket,
										   bool bRsetAimSceneComponent,
										   AActor* FollowTarget, 
										   AActor* AimTarget, 
										   FName FollowSocket, 
										   FName AimSocket, 
										   USceneComponent* FollowSceneComponent, 
										   USceneComponent* AimSceneComponent,
										   float InBlendTime, 
										   enum EViewTargetBlendFunction InBlendFunc,
										   float InBlendExp,
										   bool bInLockOutgoing,
										   bool bInIsTransitory = false,
										   float InLifeTime = 0.0f,
										   bool bInPreserveState = false);

	/** Get default blend time. */
	float GetDefaultBlendTime() { return DefaultBlendTime; }

	/** Get default blend function */
	TEnumAsByte<EViewTargetBlendFunction> GetDefaultBlendFunc() { return DefaultBlendFunc; }

	/** Get default blend exp. */
	float GetDefaultBlendExp() { return DefaultBlendExp; }

	/** Get default lock outgoing. */
	bool GetDefaultLockOutgoing() { return bDefaultLockOutgoing; }

	/** Get default preserve state. */
	bool GetDefaultPreserveState() { return bDefaultPreserveState; }

	/** Get parent camera. */
	AActor* GetParentCamera() { return ParentCamera; }

	/** Set default blend time. */
	void SetDefaultBlendTime(float InDefaultBlendTime) { DefaultBlendTime = InDefaultBlendTime; }

	/** Set default blend function. */
	void SetDefaultBlendFunc(TEnumAsByte<EViewTargetBlendFunction> InBlendFunc) { DefaultBlendFunc = InBlendFunc; }

	/** Set default blend exp. */
	void SetDefaultBlendExp(float InDefaultBlendExp) { DefaultBlendExp = InDefaultBlendExp; }

	/** Set default lock outgoing. */
	void SetDefaultLockOutgoing(bool InDefaultLockOutgoing) { bDefaultLockOutgoing = InDefaultLockOutgoing; }

	/** Set default preserve state. */
	void SetDefaultPreserveState(bool InDefaultPreserveState) { bDefaultPreserveState = InDefaultPreserveState; }

	/** Set parent camera. */
	void SetParentCamera(AActor* InCamera) { ParentCamera = InCamera; }

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EndViewTarget(APlayerController* PC) override;

	/** Reset variables when necessary. Particularly useful for automatically calling cameras. */
	void ResetOnBecomeViewTarget(bool InputbIsTransitory, float InputLifeTime, bool InputbPreserveState, AActor* InputParentCam = nullptr);

	/** Zero parameter version of ECameraManager::AddCamera. */
	UFUNCTION()
	void AddCamera();

	/** Zero parameter version of ECameraManager::DestroyCamera. */
	UFUNCTION()
	void DestroyCamera();

	/** Destroy self. UFUNCTION version of AActor::Destroy. */
	UFUNCTION()
	void DestroySelf();
};
