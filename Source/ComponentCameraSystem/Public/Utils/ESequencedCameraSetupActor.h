// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/ECameraTypes.h"
#include "ESequencedCameraSetupActor.generated.h"

class AECameraBase;

/** This class is used to collect and play a series of sub-cameras in order with their corresponding blending setups. */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AESequencedCameraSetupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AESequencedCameraSetupActor();

protected:
	/** Sequence of cameras to call. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SequencedCameraExtension")
	TArray<FECameraClassWithTargetsAndBlending> CameraSequence;

private:
	/** Index of current active camera. */
	int CameraIndex;
	/** Remaining life time of current active camera. */
	float RemainingTime;
	/** Current active camera. */
	AECameraBase* ActiveCamera;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateCamera();
};
