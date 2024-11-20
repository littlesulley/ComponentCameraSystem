// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "AnimatedCameraExtension.generated.h"

class UAnimSequence;
class UAnimDataModel;
class IAnimationDataController;

/**
 * This extension drives camera based on a a curved animation sequence. It will automatically terminates when animation finishes.
 * The animation sequence is only  used to drive cameras, so it should contain and only contain four tracks: position, rotation, scale and FOV.
 * You must ensure the input animation is valid in this regard.
 */
UCLASS(Blueprintable, BlueprintType, CollapseCategories, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UAnimatedCameraExtension : public UECameraExtensionBase
{
	GENERATED_BODY()

public:
	UAnimatedCameraExtension();

protected:
	/** The animation sequence you want to play on camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimatedCameraExtension")
	UAnimSequence* AnimToPlay;

	/** In which actor's local space you want to play the camera animation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimatedCameraExtension")
	TSoftObjectPtr<AActor> CoordinateActor;

	/** In which reference frame you want to play the camera animation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimatedCameraExtension", meta = (EditCondition = "RefCoordinateActor == nullptr"))
	FTransform Coordinate;

	/** Position offset, in reference space. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimatedCameraExtension")
	FVector PositionOffset;

	float ElapsedTime;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	void SetAnim(UAnimSequence* NewAnim) { if (NewAnim) AnimToPlay = NewAnim; }
	void SetRefActor(AActor* NewActor) { if (NewActor) CoordinateActor = NewActor; }
	void SetRef(FTransform NewTransform) { Coordinate = NewTransform; }
	void SetOffset(FVector NewOffset) { PositionOffset = NewOffset; }

private:
	void EvaluateFOV();
};
