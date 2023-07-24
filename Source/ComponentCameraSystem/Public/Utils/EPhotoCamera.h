// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EPhotoCamera.generated.h"

class UCameraComponent;
class AECameraBase;
class AEPlayerCameraManager;

/** This class is camera activated in the photo mode. 
 *  An AEPlayerCameraManager manager is mandatory.
 */
UCLASS(NotBlueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEPhotoCamera : public APawn
{
	GENERATED_BODY()

public:
	AEPhotoCamera();

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|Common", meta = (ClampMin = "10.0", ClampMax = "150.0"))
	float FOV;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|Common", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float Roll;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|DOF", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float FocusDistance;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|DOF", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Aperture;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|DOF", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SensorWidth;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|Light", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Brightness;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|Light", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Temperature;

	UPROPERTY(EditAnywhere, Category = "EPhotoMode|Color", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Intensity;

protected:
	UPROPERTY(EditAnywhere, Category = "EPhotoMode")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EPhotoMode")
	class AEPlayerCameraManager* PlayerCameraManager;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeMoveAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeRotateAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	float MoveSpeedMultiplier = 4.0f;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	float PhotoModeMaxRadius = 1200.0f;

private:
	AActor* PivotActor;
	APawn* ControlledPawn;
	AECameraBase* ActiveCamera;

protected:
	virtual void BeginPlay() override;

	/** Called for PhotoModeAction. */
	void PhotoModeSwitch(const FInputActionValue& Value);

	/** Called for PhotoModeMoveAction. */
	void PhotoModeMove(const FInputActionValue& Value);

	/** Called for PhotoModeRotateAction. */
	void PhotoModeRotate(const FInputActionValue& Value);

public:	
	virtual void BecomeViewTarget(class APlayerController* PC) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	AActor* GetPivotActor() { return PivotActor; }

private:
	void Intialize();

	UFUNCTION()
	void DelayPossess();

	UFUNCTION()
	void DelaySetViewTarget();
};
