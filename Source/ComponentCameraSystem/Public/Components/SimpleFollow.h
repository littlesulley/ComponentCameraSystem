// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ECameraComponentFollow.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraDamper.h"
#include "SimpleFollow.generated.h"


/**
 * SimpleFollow keeps a fixed offset of the camera relative to follow target.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API USimpleFollow : public UECameraComponentFollow
{
	GENERATED_BODY()

public:
	USimpleFollow();

protected:
	/** How do you want to follow the target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimpleFollow")
	ESimpleFollowType FollowType;

	/** Masks determining which axis is ignored to track the target actor. In world space.
	 *  If mask is 0, this axis is ignored. If mask is non-zero, this axis will be tracked.  
	 *	If you set any element of this property as zero, you should:
	 *  (1) Keep FollowType as WorldSpace,
	 *  (2) Pass in the sum of the follow target's location and the FollowOffset to SpawnLocation in CallCamera as an initializer.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SimpleFollow", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "FollowType == ESimpleFollowType::WorldSpace"))
	FVector AxisMasks;

	/** Damper you want to use for damping. If this is None, no damping will be applied. */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "SimpleFollow")
	TObjectPtr<UECameraVectorDamper> Damper;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;

	/** Get the real follow location. */
	FVector GetRealFollowPosition(bool bWithOffset) override;

	/** Damp temporary delta position. */
	FVector DampDeltaPosition(const FVector& TempDeltaPosition, float DeltaTime);

	/** Apply axis masks. */
	FVector ApplyAxisMask(const FVector& DampedDeltaPosition);
};
