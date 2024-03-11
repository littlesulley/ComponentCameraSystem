// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/ECameraExtensionBase.h"
#include "MixingCameraExtension.generated.h"

/**
 * This extension is used to drive mixing cameras. The final camera state
 * is determined by weighted sub-cameras added to this extension.
 * If you want to override the location or rotation derived from this extension,
 * please specify a new follow component or aim component and re-order to have
 * this extension being executed after all other components.
 */
UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UMixingCameraExtension : public UECameraExtensionBase
{
	GENERATED_BODY()

public:
	UMixingCameraExtension();

protected:
	/** List of cameras types that will be instantiated, along with optional follow target and aim target. Cannot be changed once instantiated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MixingCameraExtension")
	TArray<FECameraClassWithTargets> CameraClasses;

	/** List of weights controlling each camera's contribution. Each weight can be arbitrary value,
	 *  but it's recommended to normalize the weights and update them in blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MixingCameraExtension", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	TArray<float> Weights;

	/** Schemes to update mixing camera's weights. Cannot be changed once instantiated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MixingCameraExtension")
	EMixingCameraWeightUpdateScheme WeightUpdateScheme;

	/** Schemes to mix camera's position and rotation. Cannot be changed once instantiated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MixingCameraExtension")
	EMixingCameraMixScheme MixScheme;

	/** Methods to mix camera's rotations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MixingCameraExtension")
	EMixingCameraMixRotationMethod MixRotationMethod;

private:
	/** List of camera instances contributing to the final camera state. */
	TArray<AECameraBase*> Cameras;

public:
	virtual void UpdateComponent_Implementation(float DeltaTime) override;
	virtual void ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState) override;
	virtual void ResetOnEndViewTarget(APlayerController* PC) override;

	/** Returns a sub-camera with given class. Returns nullptr if not found or invalid.
	 * Note that inherited classes will also be considered as desired. That is, if current sub-camera is B inherited from A,
	 * and your specified camera class is A, this function will return this sub-camera as **it is** a type of A.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|MixingCamera", meta = (DeterminesOutputType = "CameraClass"))
	AECameraBase* GetSubCameraOfClass(TSubclassOf<AECameraBase> CameraClass);

	/** Returns a sub-camera at specified index. This function will check if it's valid.
	 * If the input index is out of range, it will return nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|MixingCamera")
	AECameraBase* GetSubCameraOfIndex(int index);

	/** Normalize an array of input weights. Make sure the weight number is equal to the number of sub-cameras.
	 * Besides, this function will not take invalid cameras into account. If no valid camera is found, it will return the input weight array.
	 * If all input weights are zero, it will output an average weight array.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|MixingCamera")
	TArray<float> NormalizeWeights(TArray<float> InWeights);

	/** Set weights by given input weights. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|MixingCamera")
	void SetWeights(TArray<float> InWeights);

	/** Get cameras. */
	TArray<AECameraBase*> GetCameras() { return Cameras; }

private:
	int RefreshWeights();
	FVector GetWeightedPosition();
	FRotator GetWeightedRotation();
	void UpdateWeightedRotation(float& TotalWeight, FRotator& Rotation, float CurrentWeight, AECameraBase* Camera);
	FQuat AverageRotations();
	FRotator CircularAverageRotations();
	FVector4 FindEigenvectorUsingPI(const FMatrix& M, const FVector4& V, const int Steps, const float Epsilon = 1e-5f);
	FVector4 NormalizeVector4(const FVector4& V, float Tolerance = 1e-5f);
};
