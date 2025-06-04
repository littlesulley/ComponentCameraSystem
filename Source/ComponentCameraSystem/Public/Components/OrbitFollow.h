// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/ECameraDamper.h"
#include "Components/ECameraComponentFollow.h"
#include "Kismet/KismetMathLibrary.h"
#include "OrbitFollow.generated.h"

class UControlAim;

/**
 * OrbitFollow cameras tracks the target in orbit defined at different heights.
 * You can define multiple orbits so that camera will move along the surface formed by these orbits.
 * NOTE: When you use this component for follow, you are supposed to use ControlAim component for aim
 *	     so as to control the point of view around the orbits.
 *		 If you are not using ControlAim, this component will do nothing but only follows the follow target.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UOrbitFollow : public UECameraComponentFollow
{
	GENERATED_BODY()
	
public:
	UOrbitFollow();

protected:
	/** Defined orbits. 
	 *  NOTE: orbits *MUST* be placed in a descending order of height.
	 *  For example, if you have two orbits respectively with height 1.0 (Orbit A) and 2.0 (Orbit B), 
	 *  then you should place Orbit B at index 0 and Orbit A at index 1 in the array.
	 *  Orbits should satisfy: Orbits[0].Height >= Orbits[1].Height >= Orbits[2].Height >= ...
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrbitFollow")
	TArray<FOrbit> Orbits;

	/** The function used to blend between orbits. Default is Linear. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OrbitFollow")
	TEnumAsByte<EEasingFunc::Type> BlendFunction;

	/** Damper you want to use for damping. If this is None, no damping will be applied. */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "OrbitFollow")
	TObjectPtr<UECameraVectorDamper> Damper;

	/** Cached delta residual. For some damping algorithm. */
	FVector DeltaResidual;
	/** Previous camera residual. For some damping algorithms. */
	FVector PreviousResidual;
	/** Exact spring velocity. */
	FVector SpringVelocity;
	/** Current camera root position. */
	FVector CurrentRootPosition;
	/** Cached camera root position. Used for spring damping. */
	FVector CachedRootPosition;
	/** Cached camera pitch. */
	float CachedPitch;
	/** Current camera height. */
	float Height;
	/** Current camera radius. */
	float Radius;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** Get orbits. */
	TArray<FOrbit>& GetOrbits() { return Orbits; }

	/** Get damped delta position. */
	FVector DampDeltaPosition(const FVector& TempDeltaPosition, float DeltaTime);

	/** Update height according to input DeltaY. */
	void UpdateHeight(UControlAim* ControlAimComponent);

	/** Constrain height. */
	float ConstrainHeight(float InputHeight);

	/** Get radius according to height. */
	void EvaluateRadius();

	/** Get world space offset. */
	FVector GetOrbitOffset();
};
