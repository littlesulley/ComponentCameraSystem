// Copyright 2023 by Sulley. All Rights Reserved.


#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraManager.h"
#include "Extensions/AnimatedCameraExtension.h"
#include "Extensions/KeyframeExtension.h"
#include "Extensions/MixingCameraExtension.h"
#include "Components/ECameraComponentFollow.h"
#include "Components/ECameraComponentAim.h"
#include "Utils/ECameraLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

AECameraBase::AECameraBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;

	/** Default camera component settings. */
	GetCameraComponent()->bConstrainAspectRatio = false;

	/** Create a default UECameraSettingsComponent component. */
	CameraSettingsComponent = CreateDefaultSubobject<UECameraSettingsComponent>(TEXT("ECameraSettingsComponent"));
	CameraSettingsComponent->SetupAttachment(RootComponent);

	DefaultFOV = 90.f;
	DefaultBlendTime = 1.0f;
	DefaultBlendFunc = EViewTargetBlendFunction::VTBlend_EaseOut;
	DefaultBlendExp = 1.5f;
	bDefaultLockOutgoing = true;
	bDefaultPreserveState = true;

	bIsTransitory = false;
	LifeTime = 0.0f;
	ElaspedTimeOnViewTarget = 0.0f;
	ParentCamera = nullptr;
}

void AECameraBase::BeginPlay()
{
	Super::BeginPlay();
}

void AECameraBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetSettingsComponent()->IsActive())
	{
		if (bIsTransitory)
		{
			ElaspedTimeOnViewTarget += DeltaTime;
			if (ElaspedTimeOnViewTarget >= LifeTime)
			{
				GetSettingsComponent()->ECameraManager->TerminateActiveCamera();
			}
		}
	}
}

void AECameraBase::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
	CameraSettingsComponent->BecomeViewTarget(PC, bPreserveState);
	AddCamera();

	ElaspedTimeOnViewTarget = 0.0f;
}

void AECameraBase::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);
	CameraSettingsComponent->EndViewTarget(PC);

	/** @TODO: what about using APlayerCameraManager::OnBlendComplete() ? */
	if (IsValid(GetSettingsComponent()))
	{
		/** This boolean means whether or not this camera should be destroyed. This only happens when one of the following conditions is satisfied:
		 *  1). The new camera is not transitory.
		 *  2). This camera is transitory.
		 */
		bool bSetTimerDelegate = false;

		float BlendTime = GetSettingsComponent()->GetPlayerCameraManager()->BlendTimeToGo;

		AActor* NewViewTarget = GetSettingsComponent()->GetPlayerCameraManager()->PendingViewTarget.Target;
		AECameraBase* NewViewTargetCamera = Cast<AECameraBase>(NewViewTarget);

		/** Has pending view target (blending time > 0), check its transitory. */
		if (IsValid(NewViewTargetCamera))
		{
			if (!NewViewTargetCamera->bIsTransitory || bIsTransitory)
			{
				bSetTimerDelegate = true;
			}
		}
		/** Else (blending time == 0), check the current view target's transitory. */
		else
		{
			NewViewTarget = GetSettingsComponent()->GetPlayerCameraManager()->ViewTarget.Target;
			NewViewTargetCamera = Cast<AECameraBase>(NewViewTarget);

			if (IsValid(NewViewTargetCamera) && (!NewViewTargetCamera->bIsTransitory || bIsTransitory))
			{
				bSetTimerDelegate = true;
			}
		}

		/** If the two cameras have different parent camera, destory this camera's parent. */
		if (IsValid(ParentCamera) && IsValid(NewViewTargetCamera) && (ParentCamera != NewViewTargetCamera->ParentCamera))
		{
			ParentCamera->Destroy();
		}

		if (bSetTimerDelegate)
		{
			if (BlendTime <= 0)
			{
				DestroyCamera();
			}
			else
			{
				FTimerDynamicDelegate DestroyCameraDelegate;
				DestroyCameraDelegate.BindUFunction(this, FName("DestroyCamera"));
				UKismetSystemLibrary::K2_SetTimerDelegate(DestroyCameraDelegate, BlendTime, false);
			}
		}

		/** If this camera is a mixing camera, destroy its sub-cameras. */
		UMixingCameraExtension* MixingExtension = Cast<UMixingCameraExtension>(GetExtensionOfClass(UMixingCameraExtension::StaticClass()));
		if (IsValid(MixingExtension))
		{
			TArray<AECameraBase*> Cameras = MixingExtension->GetCameras();

			for (AECameraBase* Camera : Cameras)
			{
				if (!IsValid(Camera))
				{
					continue;
				}

				if (BlendTime <= 0)
				{
					Camera->DestroySelf();
				}
				else
				{
					FTimerDynamicDelegate DestroyDelegate;
					DestroyDelegate.BindUFunction(Camera, FName("DestroySelf"));
					UKismetSystemLibrary::K2_SetTimerDelegate(DestroyDelegate, BlendTime, false);
				}
			}
		}
	}
}

void AECameraBase::ResetOnBecomeViewTarget(bool InputbIsTransitory, float InputLifeTime, bool InputbPreserveState, AActor* InputParentCam) 
{
	bIsTransitory = InputbIsTransitory;
	LifeTime = InputLifeTime;
	bPreserveState = InputbPreserveState;
	ParentCamera = InputParentCam;

	/** The length of cameras with these extensions is pre-defined. */
	if (IsValid(GetExtensionOfClass(UAnimatedCameraExtension::StaticClass())) || IsValid(GetExtensionOfClass(UKeyframeExtension::StaticClass())))
	{
		bIsTransitory = true;
		LifeTime = 99999.f;
	}
}

void AECameraBase::AddCamera()
{
	if (IsValid(CameraSettingsComponent))
	{
		CameraSettingsComponent->ECameraManager->AddCamera(this);
	}
}

void AECameraBase::DestroyCamera()
{
	if (IsValid(CameraSettingsComponent))
	{
		CameraSettingsComponent->ECameraManager->DestroyCamera(this);
	}
}

void AECameraBase::DestroySelf()
{
	Destroy();
}

AECameraBase* AECameraBase::SetFollowAndAimWithBlend(
				bool bResetFollowTarget,
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
				bool bInIsTransitory,
				float InLifeTime,
				bool bInPreserveState
			)
{
	UClass* CameraClass = nullptr;
	FVector                   SpawnLocation    = GetActorLocation();
	FRotator                  SpawnRotation    = GetActorRotation();
	AActor*                   InFollowTarget   = GetFollowTarget();
	AActor*                   InAimTarget      = GetAimTarget();
	FName                     InFollowSocket   = GetFollowComponent() ? GetFollowComponent()->GetFollowSocket()   : FName("None");
	FName                     InAimSocket      = GetAimComponent()    ? GetAimComponent()->GetAimSocket()         : FName("None");
	USceneComponent*          InFSC			   = GetFollowComponent() ? GetFollowComponent()->GetSceneComponent() : nullptr;
	USceneComponent*          InASC			   = GetAimComponent()    ? GetAimComponent()->GetSceneComponent()    : nullptr;

	if (bResetFollowTarget)           InFollowTarget = FollowTarget;
	if (bResetFollowSocket)           InFollowSocket = FollowSocket;
	if (bResetFollowSceneComponent)   InFSC          = FollowSceneComponent;
	if (bResetAimTarget)              InAimTarget    = AimTarget;
	if (bResetAimSocket)              InAimSocket    = AimSocket;
	if (bRsetAimSceneComponent)       InASC          = AimSceneComponent;

	if (this->IsA<UBlueprint>()) // @TODO: Is this correct?
	{
		CameraClass = Cast<UClass>(Cast<UBlueprint>(this)->GeneratedClass);
	}
	else
	{
		CameraClass = this->GetClass();
	}

	return UECameraLibrary::InternalCallCamera(
		this,
		CameraClass,
		SpawnLocation,
		SpawnRotation,
		InFollowTarget,
		InAimTarget,
		InFollowSocket,
		InAimSocket,
		InFSC,
		InASC,
		InBlendTime,
		InBlendFunc,
		InBlendExp,
		bInLockOutgoing,
		bInIsTransitory,
		InLifeTime,
		bInPreserveState,
		ParentCamera
	);
}
