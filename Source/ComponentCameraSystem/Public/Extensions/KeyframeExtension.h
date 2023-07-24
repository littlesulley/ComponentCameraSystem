// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Utils/ECameraTypes.h"
#include "KeyframeExtension.generated.h"

class UActorSequenceComponent;
class UMovieSceneSequence;
class UActorSequencePlayer;
class UActorSequence;

/**
 * This extension enables users to keyframe camera motion in editor and 
 * play back the curve generated via these keyframes at runtime.
 * It simply leverages the built-in ActorSequenceComponent component.
 * In addition, this extension provides the funtionality of procedural generation, 
 * i.e., randomly generating curves based the curves stored in the ActorSequence component.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UKeyframeExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UKeyframeExtension();

protected:
	/** A set of parameters controlling the generation of camera motions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	FPCMGParams PCMGParams;

	/** Override camera location to make it always track the specified actor in its local space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	TSoftObjectPtr<AActor> LocationOverride;

	/** Location offset applied in the specified actor's local space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	FVector LocationOffset;

	/** Whether to use the specified actor's rotation to override camera rotation on which the final camera rotation is based. 
	 *  For example, if the overrided rotation is 90 degree in yaw, then all keyframes will rotate 90 degs in yaw.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	TSoftObjectPtr<AActor> RotationOverride;

	/** Make the camera always look at the specified actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	TSoftObjectPtr<AActor> AimOverride;

private:
	UActorSequenceComponent* ActorSequenceComponent;
	UActorSequence* MovieSequence;
	UActorSequencePlayer* SequencePlayer;

	FFrameTime ElapsedFrames = 0;
	float ElapsedTime = 0;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

private:
	void Initialize();
};
