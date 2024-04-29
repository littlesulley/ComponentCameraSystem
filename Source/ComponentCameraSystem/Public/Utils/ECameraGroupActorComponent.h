// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utils/ECameraTypes.h"
#include "ECameraGroupActorComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraGroupActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UECameraGroupActorComponent();

public :
	/** A set of target actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor")
	TArray<FBoundingWrappedActor> TargetActors;

	/** Which method you want to use to calculte group actor's location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor")
	EGroupLocationMethod GroupLocationMethod;

	/** Anchor actor whose location is used to get group actor location if using EGroupRotationMethod::DistanceBased|Cloeset|Farest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "GroupLocationMethod == EGroupLocationMethod::UnweightedDistanceBased || GroupLocationMethod == EGroupLocationMethod::WeightedDistanceBased || GroupLocationMethod == EGroupLocationMethod::Closest || GroupLocationMethod == EGroupLocationMethod::Farest"))
	TSoftObjectPtr<AActor> AnchorLocationActor;

	/** Anchor location used to get group actor location if using EGroupRotationMethod::DistanceBased|Cloeset|Farest and AnchorLocationActor is null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "(GroupLocationMethod == EGroupLocationMethod::UnweightedDistanceBased || GroupLocationMethod == EGroupLocationMethod::WeightedDistanceBased || GroupLocationMethod == EGroupLocationMethod::Closest || GroupLocationMethod == EGroupLocationMethod::Farest) && AnchorLocationActor == nullptr"))
	FVector AnchorLocation;

	/** Given actor whose realtime location used to get group actor location if using EGroupRotationMethod::Specified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "GroupLocationMethod == EGroupLocationMethod::Specified"))
	TSoftObjectPtr<AActor> SpecifiedLocationActor;

	/** Given location used to get group actor location if using EGroupLocationMethod::Specified and SpecifiedLocationActor is null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "GroupLocationMethod == EGroupLocationMethod::Specified && SpecifiedLocationActor == nullptr"))
	FVector SpecifiedLocation;

	/** Which method you want to use to calculte group actor's rotation. Strong recommending using Specified as other methods may cause unexpected results. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor")
	EGroupRotationMethod GroupRotationMethod;

	/** Given actor whose realtime rotation is used to get group actor rotation if using EGroupRotationMethod::Specified. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "GroupRotationMethod == EGroupRotationMethod::Specified"))
	TSoftObjectPtr<AActor> SpecifiedRotationActor;

	/** Given rotation used to get group actor rotation if using EGroupRotationMethod::Specified and SpecifiedRotationActor is null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ECamera|GroupActor", meta = (EditCondition = "GroupRotationMethod == EGroupRotationMethod::Specified && SpecifiedRotationActor == nullptr"))
	FRotator SpecifiedRotation;

public:	
	/** Calculate group actor location according to GroupLocationMethod. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "ECamera|GroupActor")
	FVector GetGroupActorLocation();
	FVector GetGroupActorLocation_Implementation();

	/** Calculate group actor rotation according to GroupRotationMethod. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "ECamera|GroupActor")
	FRotator GetGroupActorRotation();
	FRotator GetGroupActorRotation_Implementation();

	/** Calculate average location. If bIsArithemetic is true, this function will return arithmetic average. Otherwise it returns weighed average. Used for GroupLocationMethod::ArithmeticAverage and GroupLocationMethod::WeightedAverage. */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	FVector GetAverageLocation(bool bIsArithmetic);

	/** Calculate average rotation. If bIsArithemetic is true, this function will return arithmetic average. Otherwise it returns weighed average. Used for EGroupRotationMethod::ArithmeticAverage and EGroupRotationMethod::WeightedAverage. */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	FRotator GetAverageRotation(bool bIsArithmetic);

	/** Check whether all weights are zero. */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	bool CheckWeightAllZero();

	/** Get the sum of weights.  */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	float GetNormalizer();

	/** Get farest or cloeset location among all target actors based on the anchor location. Used for EGroupLocationMethod::Closest and EGroupLocationMethod::Farest. */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	FVector GetFarestOrClosestLocation(bool bIsFarest);

	/** Get location based on the distance between anchor location and each target actor's location. Used for EGroupLocationMethod::UnweightedDistanceBased and EGroupLocationMethod::WeightedDistanceBased. */
	UFUNCTION(BlueprintPure, Category = "ECamera|GroupActor")
	FVector GetLocationBasedOnDistance(bool bIsWeighed);

	/** Set the weight of a `FBoundingWrappedActor` actor. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|GroupActor")
	void SetBoundingActorWeight(UPARAM(ref) FBoundingWrappedActor& BoundingWrappedActor, float InWeight) { BoundingWrappedActor.SetWeight(InWeight); }

	/** Set the width of a `FBoundingWrappedActor` actor. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|GroupActor")
	void SetBoundingActorWidth(UPARAM(ref) FBoundingWrappedActor& BoundingWrappedActor, float InWidth) { BoundingWrappedActor.SetWidth(InWidth); }

	/** Set the height of a `FBoundingWrappedActor` actor. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|GroupActor")
	void SetBoundingActorHeight(UPARAM(ref) FBoundingWrappedActor& BoundingWrappedActor, float InHeight) { BoundingWrappedActor.SetHeight(InHeight); }

	/** Set the bExcludeBoundingBox of a `FBoundingWrappedActor` actor. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|GroupActor")
	void SetBoundingActorExcludeBoundingBox(UPARAM(ref) FBoundingWrappedActor& BoundingWrappedActor, bool bInExcludeBoundingBox) { BoundingWrappedActor.SetExcludeBoundingBox(bInExcludeBoundingBox); }

	/** Add a target actor. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|GroupActor")
	void AddTargetActor(UPARAM(ref) FBoundingWrappedActor& InTargetActor) { TargetActors.Add(InTargetActor); }

private:
	void UpdateWeightedRotation(float& TotalWeight, FRotator& Rotation, float CurrentWeight, AActor* Camera);
};
