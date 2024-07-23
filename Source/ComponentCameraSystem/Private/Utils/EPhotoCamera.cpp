// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/EPhotoCamera.h"
#include "Camera/CameraComponent.h"
#include "Core/EPlayerCameraManager.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSubsystem.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Slate/SceneViewport.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Blueprint/UserWidget.h"

AEPhotoCamera::AEPhotoCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereComponent;

	SphereComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
}

void AEPhotoCamera::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PhotoModeMappingContext, 0);
		}
	}
}

void AEPhotoCamera::BecomeViewTarget(class APlayerController* PC)
{
	PlayerCameraManager = Cast<AEPlayerCameraManager>(UGameplayStatics::GetActorOfClass(this, AEPlayerCameraManager::StaticClass()));

	if (IsValid(PlayerCameraManager))
	{
		PivotPosition = PlayerCameraManager->GetPhotoModePivotPosition();
		ControlledPawn = PlayerCameraManager->GetPhotoModeControlledPawn();
		SetActorLocation(PlayerCameraManager->GetCameraLocation());
		SetActorRotation(PlayerCameraManager->GetCameraRotation());
	}
}

void AEPhotoCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUnpaused)
	{
		ElapsedPauseTime += DeltaTime;
		if (ElapsedPauseTime >= 0.6f)
		{
			PlayerCameraManager->UnpauseGame();

			APlayerController* PC = Cast<APlayerController>(GetController());
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())->RemoveMappingContext(PhotoModeMappingContext);
			PC->Possess(ControlledPawn);
			PC->SetViewTargetWithBlend(UECameraLibrary::GetActiveCamera(this), 0.0f);

			bUnpaused = false;
			this->Destroy();
		}
	}
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

void AEPhotoCamera::PhotoModeSwitch(const FInputActionValue& Value)
{
	/** Destroy ui widget.*/
	PhotoModeUI->RemoveFromParent();
	PhotoModeUI = nullptr;

	bUnpaused = true;
	PlayerCameraManager->EasyStartCameraFade(0.0f, 1.0f, 0.5f, EEasingFunc::Linear, 0.2f, 0.5f, EEasingFunc::Linear, FLinearColor::Black);
	OnPhotoModeQuit(PhotoModeUI);
}

void AEPhotoCamera::PhotoModeMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(PlayerCameraManager))
	{
		FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
		FVector CurrentCameraLocation = PlayerCameraManager->GetCameraLocation();

		// bSweep must be set to true to enable collision
		AddActorWorldOffset(UKismetMathLibrary::GetRightVector(FRotator(CameraRotation.Pitch, CameraRotation.Yaw, 0.0f)) * MovementVector.X * MoveSpeedMultiplier, true);
		AddActorWorldOffset(UKismetMathLibrary::GetForwardVector(FRotator(CameraRotation.Pitch, CameraRotation.Yaw, 0.0f)) * MovementVector.Y * MoveSpeedMultiplier, true);

		if (UKismetMathLibrary::Vector_Distance(PivotPosition, GetActorLocation()) > PhotoModeMaxRadius)
		{
			FVector Direction = GetActorLocation() - PivotPosition;
			UKismetMathLibrary::Vector_Normalize(Direction);

			SetActorLocation(PivotPosition + PhotoModeMaxRadius * Direction);
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

	/** Do something when screenshot is taken. */
	OnPhotoTaken(PhotoModeUI);
}

void AEPhotoCamera::PhotoModeTakeShot()
{
	PhotoModeShot(FInputActionValue());
}

void AEPhotoCamera::PhotoModeQuit()
{
	PhotoModeSwitch(FInputActionValue());
}