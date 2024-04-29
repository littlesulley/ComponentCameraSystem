// Copyright 2023 by Sulley. All Rights Reserved.

#include "Core/ECameraManager.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Utils/ECameraLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

AECameraManager::AECameraManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AECameraManager::AddCamera(AECameraBase* Camera)
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

void AECameraManager::DestroyCamera(AECameraBase* Camera)
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

bool AECameraManager::TerminateActiveCamera()
{
	RefreshContainer();

	/** If no valid outgoing camera exists, do nothing. */
	if (CameraContainer.Num() < 2) return false;

	int ValidCameraIndex = CameraContainer.Num() - 2;
	CameraContainer.Swap(ValidCameraIndex, ValidCameraIndex + 1);
	ActiveCamera = CameraContainer.Top();

	/** Set new view target. */
	APlayerController* PC = ActiveCamera->GetSettingsComponent()->GetPlayerController();
	PC->SetViewTargetWithBlend(ActiveCamera, ActiveCamera->DefaultBlendTime, ActiveCamera->DefaultBlendFunc, ActiveCamera->DefaultBlendExp, ActiveCamera->bDefaultLockOutgoing);
	return true;
}

void AECameraManager::RefreshContainer()
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
			--i;
			--count;
		}
	}
}