// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EPhotoCamera.generated.h"

class UCameraComponent;
class AECameraBase;
class AEPlayerCameraManager;
class UUserWidget;

/** This class is camera activated in the photo mode. 
 *  An AEPlayerCameraManager manager is mandatory.'
 *  When enabling this camera, make sure current active camera has a valid follow target.
 */
UCLASS(NotBlueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEPhotoCamera : public APawn
{
	GENERATED_BODY()

public:
	AEPhotoCamera();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPhotoMode")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EPhotoMode")
	class AEPlayerCameraManager* PlayerCameraManager;

protected:
	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeMoveAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeRotateAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	class UInputAction* PhotoModeShotAction;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	float MoveSpeedMultiplier = 4.0f;

	UPROPERTY(VisibleAnywhere, Category = "EPhotoMode")
	float PhotoModeMaxRadius = 1200.0f;

private:
	AActor* PivotActor;
	APawn* ControlledPawn;
	AECameraBase* ActiveCamera;
	UUserWidget* PhotoModeUI;

protected:
	virtual void BeginPlay() override;

	/** Called for PhotoModeAction. */
	void PhotoModeSwitch(const FInputActionValue& Value);

	/** Called for PhotoModeMoveAction. */
	void PhotoModeMove(const FInputActionValue& Value);

	/** Called for PhotoModeRotateAction. */
	void PhotoModeRotate(const FInputActionValue& Value);

	/** Called for PhotoModeShotAction. */
	void PhotoModeShot(const FInputActionValue& Value);

public:
	UFUNCTION(BlueprintCallable, Category = "EPhotoMode")
	void PhotoModeTakeShot();

	UFUNCTION(BlueprintCallable, Category = "EPhotoMode")
	void PhotoModeQuit();

public:	
	virtual void BecomeViewTarget(class APlayerController* PC) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	AActor* GetPivotActor() { return PivotActor; }

	void SetPhotoModeUI(UUserWidget* InPhotoModeUI) { PhotoModeUI = InPhotoModeUI; }

private:
	void Intialize();
};
