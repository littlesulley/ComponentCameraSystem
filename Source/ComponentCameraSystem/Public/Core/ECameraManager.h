// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ECameraManager.generated.h"

class UECameraSettingsComponent;
class ACameraActor;
class AECameraBase;

UCLASS(BlueprintType, Blueprintable, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AECameraManager : public AActor
{
	GENERATED_BODY()

public:
	AECameraManager();

protected:
	/** A container of all cameras in level. */
	UPROPERTY(VisibleAnywhere, Category = "ECameraManager")
	TArray<AECameraBase*> CameraContainer;

	/** Current active setting component. */
	UPROPERTY(BlueprintReadOnly, Category = "ECameraManager")
	AECameraBase* ActiveCamera;

public:
	/** Get the current active camera in level. */
	AECameraBase* GetActiveCamera() { return ActiveCamera; }

public:
	void AddCamera(AECameraBase* Camera);
	void DestroyCamera(AECameraBase* Camera);
	bool TerminateActiveCamera();
	void RefreshContainer();
};
