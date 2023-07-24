// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Actor.h"
#include "Core/ECameraSettingsComponent.h"
#include "ECameraBase.generated.h"

class USceneComponent;
class UCameraComponent;
class APlayerController;

enum EViewTargetBlendFunction;


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
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase")
	UECameraComponentFollow* GetFollowComponent() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->FollowComponent : nullptr; }

	/** Get AimComponent owned by this camera. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase")
	UECameraComponentAim* GetAimComponent() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->AimComponent : nullptr; }

	/** Get the CameraComponent owned by this camera. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase")
	UCameraComponent* GetCameraComponent() { return Super::GetCameraComponent(); }

	/** Get a particular extension. Return null if no such extension is found. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase", meta = (DeterminesOutputType = "ExtensionClass"))
	UECameraExtensionBase* GetExtensionOfClass(TSubclassOf<UECameraExtensionBase> ExtensionClass) { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->GetExtensionOfClass(ExtensionClass) : nullptr; }

	/** Get follow target of this camera. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase")
	AActor* GetFollowTarget() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->FollowTarget : nullptr; }

	/** Get aim target of this camera. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraBase")
	AActor* GetAimTarget() { return CameraSettingsComponent != nullptr ? CameraSettingsComponent->AimTarget : nullptr; }

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
