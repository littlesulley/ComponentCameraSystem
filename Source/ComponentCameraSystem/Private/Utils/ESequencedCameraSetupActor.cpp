// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/ESequencedCameraSetupActor.h"
#include "Utils/ECameraLibrary.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraManager.h"
#include "Core/ECameraBase.h"
#include "Kismet/GameplayStatics.h"

AESequencedCameraSetupActor::AESequencedCameraSetupActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AESequencedCameraSetupActor::BeginPlay()
{
	Super::BeginPlay();

	CameraIndex = 0;
	RemainingTime = 0.0f;

	if (CameraSequence.Num() > 0)
	{
		UpdateCamera();
	}
}

void AESequencedCameraSetupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RemainingTime -= DeltaTime;

	if (RemainingTime > 0.0f)
	{
		return;
	}
	else
	{
		++CameraIndex;
		if (CameraIndex >= CameraSequence.Num())
		{
			if (UECameraLibrary::GetActiveCamera(this) == ActiveCamera)
			{
				UECameraLibrary::TerminateActiveCamera(this);
			}

			Destroy();
			return;
		}

		UpdateCamera();
	}
}

void AESequencedCameraSetupActor::UpdateCamera()
{
	RemainingTime = CameraSequence[CameraIndex].Duration;

	AActor* FollowTarget = nullptr;
	AActor* AimTarget = nullptr;

	if (CameraSequence[CameraIndex].FollowTarget.IsValid())
	{
		FollowTarget = CameraSequence[CameraIndex].FollowTarget.Get();
	}
	else
	{
		FollowTarget = UGameplayStatics::GetActorOfClass(this, CameraSequence[CameraIndex].FollowTargetType);
	}
	if (CameraSequence[CameraIndex].AimTarget.IsValid())
	{
		AimTarget = CameraSequence[CameraIndex].AimTarget.Get();
	}
	else
	{
		AimTarget = UGameplayStatics::GetActorOfClass(this, CameraSequence[CameraIndex].AimTargetType);
	}

	AECameraBase* Camera = UECameraLibrary::CallCamera(this,
		CameraSequence[CameraIndex].CameraClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FollowTarget,
		AimTarget,
		CameraSequence[CameraIndex].FollowSocket,
		CameraSequence[CameraIndex].AimSocket,
		CameraSequence[CameraIndex].BlendTime,
		CameraSequence[CameraIndex].BlendFunc,
		CameraSequence[CameraIndex].BlendExp,
		CameraSequence[CameraIndex].bLockOutgoing,
		true,
		99999.f,
		CameraSequence[CameraIndex].bPreserveState,
		true,
		this
	);

	ActiveCamera = Camera;
}
