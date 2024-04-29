// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "ConfinerExtension.generated.h"

class ATriggerBox;

/**
 * This extension confines camera within areas bounded by trigger boxes.
 * Ensure this component is executed after follow component and before aim component.
 * You can do this by altering the stages of these components.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UConfinerExtension : public UECameraExtensionBase
{
	GENERATED_BODY()
	
public:
	UConfinerExtension();

protected:
	/** Boxes used to define confine areas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfinerExtension")
	TArray<TSoftObjectPtr<ATriggerBox>> Boxes;

	/** Exit box damping time. You can set this relatively large, say, 1.0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfinerExtension")
	float ExitDamping;

	/** Enter box damping time. You should set this relatively small, say, 0.2. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfinerExtension")
	float EnterDamping;

	/** Cached raw location. Used to reset camera location before tick begins. */
	FVector CachedRawLocation;
	/** Cached new location. Used to record previous modified camera location. */
	FVector CachedNewLocation;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void BindToOnPreTickComponent() override;

	/** Get boxes. */
	TArray<TSoftObjectPtr<ATriggerBox>>& GetBoxes() { return Boxes; }

	/** Check whether current camera position is within bounds. */
	bool CheckIfWithinBounds();

	/** Find the nearest position on bounds if current camera position is not on bounds. */
	FVector FindNearestPositionOnBounds();

	/** Get damped delta position from current camera position to desired position. */
	FVector GetDampedDeltaPosition(const FVector& DesiredPosition, float DeltaTime, bool bWithinBox);
};
