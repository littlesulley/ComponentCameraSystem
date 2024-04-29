// Copyright 2023 by Sulley. All Rights Reserved.


#include "Utils/ECameraGroupActor.h"
#include "Utils/ECameraTypes.h"
#include "Utils/ECameraGroupActorComponent.h"
#include "Kismet/GameplayStatics.h"

AECameraGroupActor::AECameraGroupActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("USceneComponent"));
	CameraGroupActorComponent = CreateDefaultSubobject<UECameraGroupActorComponent>(TEXT("ECameraGroupActorComponent"));
	RootComponent = SceneComponent;
}

void AECameraGroupActor::Tick(float DeltaTime)
{
	SetActorLocation(CameraGroupActorComponent->GetGroupActorLocation());
	SetActorRotation(CameraGroupActorComponent->GetGroupActorRotation());
}

