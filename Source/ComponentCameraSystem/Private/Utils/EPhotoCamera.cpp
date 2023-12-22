// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/EPhotoCamera.h"
#include "Camera/CameraComponent.h"
#include "Core/EPlayerCameraManager.h"
#include "Core/ECameraManager.h"
#include "Core/ECameraBase.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Slate/SceneViewport.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Blueprint/UserWidget.h"

AEPhotoCamera::AEPhotoCamera()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	CameraComponent->SetupAttachment(RootComponent);
}

void AEPhotoCamera::BeginPlay()
{
	Super::BeginPlay();

	Intialize();
}

void AEPhotoCamera::BecomeViewTarget(class APlayerController* PC)
{
	if (IsValid(PlayerCameraManager))
	{
		PivotActor = PlayerCameraManager->GetPhotoModePivotActor();
		ControlledPawn = PlayerCameraManager->GetPhotoModeControlledPawn();
		SetActorLocation(PlayerCameraManager->GetCameraLocation());
		SetActorRotation(PlayerCameraManager->GetCameraRotation());
	}
}

void AEPhotoCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEPhotoCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Switch
		EnhancedInputComponent->BindAction(PhotoModeAction, ETriggerEvent::Completed, this, &AEPhotoCamera::PhotoModeSwitch);

		// Move
		EnhancedInputComponent->BindAction(PhotoModeMoveAction, ETriggerEvent::Triggered, this, &AEPhotoCamera::PhotoModeMove);

		// Rotate
		EnhancedInputComponent->BindAction(PhotoModeRotateAction, ETriggerEvent::Triggered, this, &AEPhotoCamera::PhotoModeRotate);

		// Shot
		EnhancedInputComponent->BindAction(PhotoModeShotAction, ETriggerEvent::Completed, this, &AEPhotoCamera::PhotoModeShot);
	}
}

void AEPhotoCamera::Intialize()
{
	AActor* CameraManagerActor = UGameplayStatics::GetActorOfClass(this, AEPlayerCameraManager::StaticClass());

	if (AEPlayerCameraManager* CameraManager = Cast<AEPlayerCameraManager>(CameraManagerActor))
	{
		PlayerCameraManager = CameraManager;

		/** Input actions. */
		PhotoModeAction = PlayerCameraManager->GetPhotoModeAction();
		PhotoModeMoveAction = PlayerCameraManager->GetPhotoModeMoveAction();
		PhotoModeRotateAction = PlayerCameraManager->GetPhotoModeRotateAction();
		PhotoModeShotAction = PlayerCameraManager->GetPhotoModeShotAction();

		/** Photo mode parameters. */
		MoveSpeedMultiplier = PlayerCameraManager->GetMoveSpeedMultiplier();
		PhotoModeMaxRadius = PlayerCameraManager->GetPhotoModeMaxRaiuds();
	}
}

void AEPhotoCamera::PhotoModeSwitch(const FInputActionValue& Value)
{
	if (IsValid(ControlledPawn))
	{
		FTimerManager* TimerManager = &GetWorld()->GetTimerManager();

		AActor* ManagerActor = UGameplayStatics::GetActorOfClass(this, AECameraManager::StaticClass());

		if (IsValid(ManagerActor))
		{
			PlayerCameraManager->EasyStartCameraFade(0.0f, 1.0f, 0.5f, EEasingFunc::Linear, 0.2f, 0.5f, EEasingFunc::Linear, FLinearColor::Black);

			ActiveCamera = Cast<AECameraManager>(ManagerActor)->GetActiveCamera();

			FTimerHandle TimeHandle;
			TimerManager->SetTimer(
				TimeHandle,
				[this]() {
					Cast<APlayerController>(GetController())->SetViewTargetWithBlend(ActiveCamera, 0.0f);
				},
				0.5f,
				false
			);
		}

		FTimerHandle TimeHandle;
		TimerManager->SetTimer(
			TimeHandle,
			[this]() {
				Cast<APlayerController>(GetController())->Possess(ControlledPawn);
			},
			0.7f,
			false
		);

		PlayerCameraManager->UnpauseGame();
	}
}

void AEPhotoCamera::PhotoModeMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(PlayerCameraManager) && IsValid(PivotActor))
	{
		FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
		FVector CurrentCameraLocation = PlayerCameraManager->GetCameraLocation();

		AddActorWorldOffset(UKismetMathLibrary::GetRightVector(FRotator(CameraRotation.Pitch, CameraRotation.Yaw, 0.0f)) * MovementVector.X * MoveSpeedMultiplier);
		AddActorWorldOffset(UKismetMathLibrary::GetForwardVector(FRotator(CameraRotation.Pitch, CameraRotation.Yaw, 0.0f)) * MovementVector.Y * MoveSpeedMultiplier);

		if (UKismetMathLibrary::Vector_Distance(PivotActor->GetActorLocation(), GetActorLocation()) > PhotoModeMaxRadius)
		{
			FVector Direction = GetActorLocation() - PivotActor->GetActorLocation();
			UKismetMathLibrary::Vector_Normalize(Direction);

			SetActorLocation(PivotActor->GetActorLocation() + PhotoModeMaxRadius * Direction);
		}
	}
}

void AEPhotoCamera::PhotoModeRotate(const FInputActionValue& Value)
{
	FVector2D RotationVector = Value.Get<FVector2D>();
	AddActorWorldRotation(FRotator(0.0f, RotationVector.X, 0.0f));
	AddActorLocalRotation(FRotator(-RotationVector.Y, 0.0f, 0.0f));
}

void AEPhotoCamera::PhotoModeShot(const FInputActionValue& Value)
{
	/** Using reflection to get resolution in widget. */
	UClass* ObjectClass = PhotoModeUI->GetClass();

	FProperty* PropertyW = FindFProperty<FProperty>(ObjectClass, FName("ResolutionWidth"));
	FProperty* PropertyH = FindFProperty<FProperty>(ObjectClass, FName("ResolutionHeight"));
	FIntProperty* IntPropertyW = CastField<FIntProperty>(PropertyW);
	FIntProperty* IntPropertyH = CastField<FIntProperty>(PropertyH);
	int Width = IntPropertyW->GetPropertyValue_InContainer(PhotoModeUI);
	int Height = IntPropertyH->GetPropertyValue_InContainer(PhotoModeUI);

	TArray<FStringFormatArg> Args;
	Args.Add(FStringFormatArg(Width));
	Args.Add(FStringFormatArg(Height));
	Args.Add(FStringFormatArg(FPaths::ProjectSavedDir()));
	Args.Add(FStringFormatArg(FDateTime::Now().ToString()));

	FString Command = FString::Format(TEXT("HighResShot {0}x{1} filename=\"{2}/HighResSnapshots/{3}.png\""), Args);
	Cast<APlayerController>(GetController())->ConsoleCommand(Command, true);
}

void AEPhotoCamera::PhotoModeTakeShot()
{
	PhotoModeShot(FInputActionValue());
}

void AEPhotoCamera::PhotoModeQuit()
{
	PhotoModeSwitch(FInputActionValue());
}