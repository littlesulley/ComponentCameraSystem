// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "UObject/NoExportTypes.h"
#include "Core/ECameraBase.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "ECameraComponentBase.generated.h"

/**
 * CameraComponentBase serves as the base for all components to inherit from.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraComponentBase : public UObject
{
	GENERATED_BODY()

public:
	UECameraComponentBase();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraComponentBase")
	EStage Stage;

	UPROPERTY(BlueprintReadOnly, Category = "CameraComponentBase")
	AActor* OwningActor;

	UPROPERTY(BlueprintReadOnly, Category = "CameraComponentBase")
	AECameraBase* OwningCamera;

	UPROPERTY(BlueprintReadOnly, Category = "CameraComponentBase")
	UECameraSettingsComponent* OwningSettingComponent;

public:
	/** Get stage at which this component is executed in the camera pipeline. */
	EStage GetStage() const { return Stage; }

	/** Set staget at which this component is executed in the camera pipeline. */
	void SetStage(EStage NewStage) { Stage = NewStage; }

	/** Get the actor that owns this component. */
	AActor* GetOwningActor() { return OwningActor; }

	/** Get the `ECameraBase` that owns this component. A cast version of `GetOwningActor`. */
	AECameraBase* GetOwningCamera() { return OwningCamera; }

	/** Get the `CameraComponent` stored in the owning actor. */
	UCameraComponent* GetCameraComponent() { return OwningCamera->GetCameraComponent(); }

	/** Get the `ECameraOwningSettingsComponent` that owns this component. */
	UECameraSettingsComponent* GetOwningSettingComponent() { return OwningSettingComponent;	}

	void SetOwningActor(AActor* NewOwningActor) { OwningActor = NewOwningActor; }
	void SetOwningCamera(AECameraBase* NewOwningCamera) { OwningCamera = NewOwningCamera; }
	void SetOwningSettingComponent(UECameraSettingsComponent* NewOwningSettingComponent) { OwningSettingComponent = NewOwningSettingComponent; }

public:
	/** Check if this component is valid. Currently is of no use. */
	virtual bool Valid();
	/** This function should be used to reset internal variables when the owning camera becomes view target. Only used internally. */
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) { }
	/** This function should be used to reset internal variables when the owning camera exits view target. Only used internally. */
	virtual void ResetOnEndViewTarget(APlayerController* PC) { }
	/** Do some cleaning work when destroyed. Currently is of no use. */
	virtual void BeginDestroy() override;

	UFUNCTION()
	virtual void BindToOnPreTickComponent();

	UFUNCTION()
	virtual void BindToOnPostTickComponent();

	UFUNCTION()
	virtual void BindToOnBecomeViewTarget(APlayerController* PC);

	UFUNCTION()
	virtual void BindToOnEndViewTarget(APlayerController* PC);

	/** You can override this function to define the behaviour of this component. This is executed every tick. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "UpdateComponent"))
	void UpdateComponent(float DeltaTime);
	virtual void UpdateComponent_Implementation(float DeltaTime) {}

	/** You can implement this function to define what this component will do before any component executes in one tick. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnPreTickComponent"))
	void K2_BindToOnPreTickComponent();

	/** You can implement this function to define what this component will do after all components execute in one tick. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnPostTickComponent"))
	void K2_BindToOnPostTickComponent();

	/** You can implement this function to define what this component will do when it becomes view target. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBecomeViewTarget"))
	void K2_BindToOnBecomeViewTarget(APlayerController* PC);

	/** You can implement this function to define what this component will do once it exits view target. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndViewTarget"))
	void K2_BindToOnEndViewTarget(APlayerController* PC);
};
