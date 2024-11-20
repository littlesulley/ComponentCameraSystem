// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECameraSubsystem.generated.h"

UENUM()
enum class ECameraWorkMode
{
	Classic,
	StateMachine
};

UCLASS()
class COMPONENTCAMERASYSTEM_API UECameraSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem

	// Set camera work mode, to be developed
	UFUNCTION(/*BlueprintCallable, */Category = "ECameraSubsystem")
	void SetCameraWorkMode(ECameraWorkMode Mode);

	class AECameraBase* GetActiveCamera() 
	{
		return ActiveCamera;
	}

	// Add a new incoming camera
	void AddCamera(class AECameraBase* Camera);

	// Destroy a given camera
	void DestroyCamera(class AECameraBase* Camera);

	// Terminate current active camera
	bool TerminateActiveCamera();

	// Refresh the camera container
	void RefreshContainer();

private:
	// Camera work mode, either Classic or StateMachine, to be developed
	UPROPERTY()
	ECameraWorkMode WorkMode;
	
	// Container holding all current and cached cameras
	TArray<TObjectPtr<class AECameraBase>> CameraContainer;

	// Current active camera
	TObjectPtr<class AECameraBase> ActiveCamera;
};
