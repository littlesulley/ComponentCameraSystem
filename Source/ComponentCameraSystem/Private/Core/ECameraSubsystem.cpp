// Copyright 2023 by Sulley. All Rights Reserved.

#include "Core/ECameraSubsystem.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Utils/ECameraLibrary.h"
#include "Kismet/GameplayStatics.h"

void UECameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	WorkMode = ECameraWorkMode::Classic;
}

void UECameraSubsystem::Deinitialize()
{

}

void UECameraSubsystem::SetCameraWorkMode(ECameraWorkMode Mode)
{
	WorkMode = Mode;
}

void UECameraSubsystem::AddCamera(class AECameraBase* Camera)
{
	if (IsValid(Camera))
	{
		int32 index = CameraContainer.Find(Camera);

		if (index != INDEX_NONE)
		{
			CameraContainer.RemoveAt(index);
		}

		CameraContainer.Push(Camera);
		ActiveCamera = Camera;
	}
}

void UECameraSubsystem::DestroyCamera(AECameraBase* Camera)
{
	if (IsValid(Camera))
	{
		int32 index = CameraContainer.Find(Camera);

		if (index != INDEX_NONE)
		{
			CameraContainer[index]->Destroy();
			CameraContainer.RemoveAt(index);
		}
	}
}

bool UECameraSubsystem::TerminateActiveCamera()
{
	RefreshContainer();

	if (CameraContainer.Num() < 2)
	{
		return false;
	}

	int ValidCameraIndex = CameraContainer.Num() - 2;
	CameraContainer.Swap(ValidCameraIndex, ValidCameraIndex + 1);
	ActiveCamera = CameraContainer.Top();

	// @TODO: Where should the PC come from? GameInstance?
	APlayerController* PC = ActiveCamera->GetSettingsComponent()->GetPlayerController();
	PC->SetViewTargetWithBlend(ActiveCamera, ActiveCamera->DefaultBlendTime, ActiveCamera->DefaultBlendFunc, ActiveCamera->DefaultBlendExp, ActiveCamera->bDefaultLockOutgoing);

	return true;
}

void UECameraSubsystem::RefreshContainer()
{
	for (int i = 0, count = CameraContainer.Num() - 1; i < count; ++i)
	{
		/** Filter out the following cameras:
		 *  1). Invalid cameras
		 *  2). Transitory cameras. The rationale is that transitory cameras are only "transitory", and they cannot resume once overwritten.
		 */
		if (!IsValid(CameraContainer[i]) || CameraContainer[i]->bIsTransitory)
		{
			if (IsValid(CameraContainer[i]) && CameraContainer[i]->bIsTransitory)
			{
				CameraContainer[i]->Destroy();
			}

			CameraContainer.RemoveAt(i);
			i--;
			count--;
		}
	}
}