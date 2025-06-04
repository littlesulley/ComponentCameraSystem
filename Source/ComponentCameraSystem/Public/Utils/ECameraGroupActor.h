// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/ECameraTypes.h"
#include "ECameraGroupActor.generated.h"

class UECameraGroupActorComponent;
class USceneComponent;

/** Default group actor, only contains a CameraGroupActorComponent.
 *  If you want to define your own group actor behaviour, you should 
 *  inherit from ECameraGroupActorComponent and override corresponding methods,
 *  and then use your customized component in place of the default component in blueprint.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AECameraGroupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AECameraGroupActor();

	/** Default group actor component. */
	UPROPERTY(VisibleAnywhere, Category = "ECameraGroupActor")
	TObjectPtr<USceneComponent> SceneComponent;

	/** Default group actor component. */
	UPROPERTY(VisibleAnywhere, Category = "ECameraGroupActor")
	TObjectPtr<UECameraGroupActorComponent> CameraGroupActorComponent;

public:	
	virtual void Tick(float DeltaTime) override;
};
