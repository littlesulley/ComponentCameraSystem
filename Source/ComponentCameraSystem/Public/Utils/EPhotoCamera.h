// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EPhotoCamera.generated.h"

class UCameraComponent;
class USphereComponent;
class AECameraBase;
class AEPlayerCameraManager;
class UUserWidget;

/** This class is camera activated in the photo mode. 
 *  An AEPlayerCameraManager manager is mandatory.'
 *  When enabling this camera, make sure current active camera has a valid follow target.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEPhotoCamera : public APawn
{
	GENERATED_BODY()

public:
	AEPhotoCamera(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPhotoMode")
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPhotoMode")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EPhotoMode")
	TObjectPtr<class AEPlayerCameraManager> PlayerCameraManager;

	/** Input mapping context for this photo camera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* PhotoModeMappingContext;

	/** Input action for switching on/off photo mode. Make sure this action is added to current input mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	class UInputAction* PhotoModeAction;

	/** Input action controlling movement of photo mode camera. Make sure this action is added to current input mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	class UInputAction* PhotoModeMoveAction;

	/** Input action controlling rotation of photo mode camera. Make sure this action is added to current input mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	class UInputAction* PhotoModeRotateAction;

	/** Input action for taking a screenshot. Make sure this action is added to current input mapping context. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	class UInputAction* PhotoModeShotAction;

	/** Camera move speed multiplier, controls how fast photo camera moves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MoveSpeedMultiplier = 4.0f;

	/** Photo mode max radius. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PhotoMode", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float PhotoModeMaxRadius = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	FVector PivotPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhotoMode")
	APawn* ControlledPawn;

private:
	UUserWidget* PhotoModeUI;
	float ElapsedPauseTime{ 0.0f };
	bool bUnpaused{ false };

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
	/** The action of taking screenshot. This functioin will also call OnPhotoTaken, which you should implement in your Photo Camera. */
	UFUNCTION(BlueprintCallable, Category = "EPhotoMode")
	void PhotoModeTakeShot();

	/** The action of quiting photo mode. */
	UFUNCTION(BlueprintCallable, Category = "EPhotoMode")
	void PhotoModeQuit();

	/** Custom event when taking screenshot. Generally, this function is used to play widget animation (as in the input PhotoModeWidget parameter) and do cleanup work. */
	UFUNCTION(BlueprintImplementableEvent, Category = "EPhotoMode", DisplayName = "OnPhotoTaken")
	void OnPhotoTaken(UUserWidget* PhotoModeWidget);

	/** Custom event when quitting photo mode. This happens before the underlying widget is destructed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "EPhotoMode", DisplayName = "OnPhotoModeQuit")
	void OnPhotoModeQuit(UUserWidget* PhotoModeWidget);

public:	
	virtual void BecomeViewTarget(class APlayerController* PC) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetPhotoModeUI(UUserWidget* InPhotoModeUI) { PhotoModeUI = InPhotoModeUI; }
};
