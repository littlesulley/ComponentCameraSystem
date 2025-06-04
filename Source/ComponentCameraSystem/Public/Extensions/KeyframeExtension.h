// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Utils/ECameraTypes.h"
#include "Channels/MovieSceneDoubleChannel.h"
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
UCLASS(Blueprintable, BlueprintType, CollapseCategories, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UKeyframeExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UKeyframeExtension();

	/** A set of parameters controlling the generation of camera motions. Currently obsolete. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	FPCMGParams PCMGParams;

	/** In which actor's local space you want to play the keyframes, including position and rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KeyframeExtension")
	TObjectPtr<AActor> CoordinateActor;

	/** CoordinateActor's socket the camera is based on. If this is specified, this socket's local space will be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension", meta = (EditCondition = "CoordinateActor != nullptr"))
	FName CoordinateSocket;

	/** In which reference coordinate you want to apply the keyframes. Requires CoordinateActor is null. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KeyframeExtension", meta = (EditCondition = "CoordinateActor == nullptr"))
	FTransform Coordinate;

	/** Whether to apply only keyframed positions in the specified CoordinateActor or Coordiante space.
	  * If true, camera's rotation will be the original keyframed rotations.
	  * Otherwise, the camera's final rotation will be changed according to the given CoordinateActor or Coordiante.
	  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KeyframeExtension")
	bool bCoordinateLocationOnly;

	/** Location offset applied to the keyframed positions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	FVector LocationOffset;

	/** Make the camera always look at the specified actor. This happens after keyframe is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension")
	TObjectPtr<AActor> AimOverride;

	/** AimOverride's socket for the camera to aim at. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyframeExtension", meta = (EditCondition = "AimOverride != nullptr"))
	FName AimSocket;

private:
	UActorSequenceComponent* ActorSequenceComponent;
	TObjectPtr<UActorSequence> MovieSequence;
	TObjectPtr<UActorSequencePlayer> SequencePlayer;

	bool bModified = false;
	TArray<FFrameNumber> RawTransformTimes[9];
	TArray<FMovieSceneDoubleValue> RawTransformValues[9];

	FFrameTime ElapsedFrames = 0;
	float ElapsedTime = 0;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** For instantiated objects. */
	void TossSequence();
	/** For CDO in editor. */
	void TossSequence(UActorSequenceComponent* InActorSequenceComponent);
	/** For instantiated objects. */
	void Recover();
	/** For CDO in editor. */
	void Recover(UActorSequenceComponent* InActorSequenceComponent);
	/** For instantiated objects. */
	void Save();
	/** For CDO in editor. */
	void Save(UActorSequenceComponent* InActorSequenceComponent);

private:
	void Initialize();
	FTransform GetCoordinateTransform();
	FVector GetAimDestLocation();
	void TossChannel(FMovieSceneDoubleChannel* Channel);
	void DuplicateRawData(TArrayView<FMovieSceneDoubleChannel*> Channels);
	TArrayView<FMovieSceneDoubleChannel*> GetTransformChannels(UActorSequence* InMovieSceneSequence);
	void ConstructInput(TArray<float>& Input, const TArrayView<FMovieSceneDoubleChannel*>& Channels);
	void ApplyOutput(TArray<float>& Output, TArrayView<FMovieSceneDoubleChannel*>& Channels, UActorSequence* InMovieSequence);
	void GetFunctionOutput(FFunctionParamsCollection FcuntionParams, TArray<float>& Output);
};
