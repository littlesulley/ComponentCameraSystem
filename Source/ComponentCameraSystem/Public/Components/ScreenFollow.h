// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/ECameraLibrary.h"
#include "Components/ECameraComponentFollow.h"
#include "ScreenFollow.generated.h"

class UControlAim;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * ScreenFollow keeps a fixed position of the follow target on screen space.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UScreenFollow : public UECameraComponentFollow
{
	GENERATED_BODY()

public:
	UScreenFollow();

protected:
	/** Camera distance to the *real* follow target after applying FollowOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	float CameraDistance;

	/** A curve used to adjust camera distance according to camera pitch. The x-axis range should be within [-90, 90]. 
	 *  When camera looks down, pitch is negative; when camera looks up, pitch is positive.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	UCurveFloat* PitchDistanceCurve;

	/** Whether to adapt camera orientation to character movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	bool bAdaptToMovement;

	/** Speed when adapting camera orientation to character movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	float AdaptToMovementSpeed;
	
	/** Zoom settings. Will override PitchDistanceCurve. Be careful. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	FZoomSettings ZoomSettings;

	/** Damp parameters you want to use for damping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	FDampParams DampParams;

	/** Screen space offset applied to the *real* follow target after applying FollowOffset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow", meta = (ClampMin = "-0.5", ClampMax = "0.5"))
	FVector2f ScreenOffset;

	/** Start and end distance in which adaptive screen offset (X axis) is enable.
	 *  Highly recommended to use the ConstrainPitchExtension and the ModifyAimPointExtension instead.
	 *  Will be deprecated in the future.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	FVector2f AdaptiveScreenOffsetDistanceX;

	/** Start and end distance in which adaptive screen offset (Y axis) is enable. 
	 *  Highly recommended to use the ConstrainPitchExtension and the ModifyAimPointExtension instead.
	 *  Will be deprecated in the future.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow")
	FVector2f AdaptiveScreenOffsetDistanceY;

	/** Width of the screen offset. Pivoted at the X axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2f ScreenOffsetWidth;

	/** Height of the screen offset. Pivoted at the Y axis position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScreenFollow", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector2f ScreenOffsetHeight;

	/** Real follow position. */
	FVector RealFollowPosition;
	/** Local space follow position. */
	FVector LocalFollowPosition;
	/** Cached delta residual. For some damping algorithm. */
	FVector DeltaResidual;
	/** Previous camera residual. For some damping algorithms. */
	FVector PreviousResidual;
	/** Velocity for exact spring. */
	FVector SpringVelocity;
	/** Cached ControlAim component. */
	UControlAim* ControlAim;
	/** Current camera distance. */
	float CurrentCameraDistance = -1.f;
	/** Cached zoom value. */
	float CachedZoomValue;
	/** Enhanched input subsystem. */
	UEnhancedInputLocalPlayerSubsystem* Subsystem;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;

	/** This function allows you set any property in Damp params of a ScreenFollow component. 
	  * You should make sure the property name is correct and matches the value type. That is, if PropertyName is DampTime, Value should be of a Vector type.
	  * If no such PropertyName exists or value type does not match, nothing will happen.
	  * @param PropertyName: A property name of DampParams, e.g., DampTime, Residual, etc.
	  * @param Value: The new value you want to set to PropertyName. 
	  */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "ScreenFollow", meta = (DisplayName = "SetPropertyInDampParams", CustomStructureParam = "Value"))
	void SetPropertyInDampParams(FName PropertyName, const int32& Value);

	DECLARE_FUNCTION(execSetPropertyInDampParams)
	{
		P_GET_PROPERTY(FNameProperty, PropertyName);

		Stack.StepCompiledIn<FProperty>(NULL);
		void* ValuePtr = Stack.MostRecentPropertyAddress;
		FProperty* ValueProperty = Stack.MostRecentProperty;

		P_FINISH;

		P_NATIVE_BEGIN;
		P_THIS->Generic_SetPropertyInDampParams(PropertyName, ValuePtr, ValueProperty);
		P_NATIVE_END;
	}

	void Generic_SetPropertyInDampParams(FName PropertyName, void* ValuePtr, FProperty* ValueProperty)
	{
		if (!IsValid(this))
		{
			UE_LOG(LogTemp, Warning, TEXT("Context target (ScreenFollow component) is not valid when calling SetPropertyInDampParams"));
			return;
		}

		// Get property of DampParams struct
		if (FStructProperty* StructProperty = CastField<FStructProperty>(UECameraLibrary::GetPropertyFromObject(this, "DampParams")))
		{
			void* StructPtr = StructProperty->ContainerPtrToValuePtr<void>(this);
			UScriptStruct* Struct = StructProperty->Struct;
			FProperty* SrcProperty = FindFProperty<FProperty>(Struct, PropertyName);

			if (SrcProperty == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot find property %s in DampParams."), *PropertyName.ToString());
				return;
			}

			void* SrcPtr = SrcProperty->ContainerPtrToValuePtr<void>(StructPtr);
			if (SrcProperty->SameType(ValueProperty))
			{
				SrcProperty->CopyCompleteValue(SrcPtr, ValuePtr);
			}
			else if (SrcProperty->IsA<FFloatProperty>() && ValueProperty->IsA<FDoubleProperty>())
			{
				CastField<FFloatProperty>(SrcProperty)->SetPropertyValue(SrcPtr, CastField<FDoubleProperty>(ValueProperty)->GetPropertyValue(ValuePtr));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("The found property %s does not has the same type as given property %s, respectively are %s and %s"),
					*SrcProperty->NamePrivate.ToString(), *ValueProperty->NamePrivate.ToString(), *SrcProperty->GetCPPType(), *ValueProperty->GetCPPType());
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find property DampParams in object %s."), *this->GetName());
			return;
		}
	}

	/** Return real follow position, equal to GetRealFollowPosition. */
	FVector GetFollowPosition() { return RealFollowPosition; }

	/** Get local space follow position. */
	FVector GetLocalSpaceFollowPosition() { return LocalFollowPosition; }

	/** Get screen offset. */
	FVector2f GetScreenOffset() { return ScreenOffset; }

	/** Get screen offset width. */
	FVector2f GetScreenOffsetWidth() { return ScreenOffsetWidth; }

	/** Get screen offset height. */
	FVector2f GetScreenOffsetHeight() { return ScreenOffsetHeight; }

	/** Get the *real* screen offset based on the distance between follow position and aim position. */
	FVector2f GetAdaptiveScreenOffset(const FVector& FollowPosition, const FVector& AimPositio);

	/** Normalize pitch into [-90, 90]. */
	float NormalizePitch(float Pitch);

	/** Set delta position along the local X axis. */
	void SetForwardDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, float RealCameraDistance);

	/** Get delta position along the local YZ plane. */
	void SetYZPlaneDelta(const FVector& LocalSpaceFollowPosition, FVector& TempDeltaPosition, const FVector2f& RealScreenOffset, float RealCameraDistance);

	/** Damp temporary delta position. */
	FVector DampDeltaPosition(const FVector& LocalSpaceFollowPosition, const FVector& TempDeltaPosition, float DeltaTime, const FVector2f& RealScreenOffset);

	/** Ensure after damping, the follow target will be within the bound. */
	void EnsureWithinBounds(const FVector& LocalSpaceFollowPosition, FVector& DampedDeltaPosition, const FVector2f& RealScreenOffset, float RealCameraDistance);

	/** Check whether has mouse input. */
	bool HasControlAimInput();

	/** Get damped zoom value. */
	float GetDampedZoomValue(const float& ZoomValue, const float& DeltaTime);
};

