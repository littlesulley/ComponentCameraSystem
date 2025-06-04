// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/ECameraGroupActorComponent.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"

UECameraGroupActorComponent::UECameraGroupActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GroupLocationMethod = EGroupLocationMethod::ArithmeticAverage;
	AnchorLocation = FVector(0, 0, 0);
	SpecifiedLocation = FVector(0, 0, 0);
	GroupRotationMethod = EGroupRotationMethod::Specified;
	SpecifiedRotation = FRotator::ZeroRotator;
}

FVector UECameraGroupActorComponent::GetGroupActorLocation_Implementation()
{
	/** Technically the length of TargetActors should not be zero. */
	if (TargetActors.Num() == 0) return FVector();

	FVector OutputLocation;
	if (GroupLocationMethod == EGroupLocationMethod::ArithmeticAverage)
	{
		OutputLocation = GetAverageLocation(true);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::WeighedAverage)
	{
		if (CheckWeightAllZero()) OutputLocation = GetAverageLocation(true);
		else OutputLocation = GetAverageLocation(false);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::UnweightedDistanceBased)
	{
		OutputLocation = GetLocationBasedOnDistance(false);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::WeightedDistanceBased)
	{
		if (CheckWeightAllZero()) OutputLocation = GetLocationBasedOnDistance(false);
		else OutputLocation = GetLocationBasedOnDistance(true);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::Closest)
	{
		OutputLocation = GetFarestOrClosestLocation(false);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::Farest)
	{
		OutputLocation = GetFarestOrClosestLocation(true);
	}
	else if (GroupLocationMethod == EGroupLocationMethod::Specified)
	{
		if (SpecifiedLocationActor) OutputLocation = SpecifiedLocationActor->GetActorLocation();
		else OutputLocation = SpecifiedLocation;
	}
	else OutputLocation = FVector();

	return OutputLocation;
}

FRotator UECameraGroupActorComponent::GetGroupActorRotation_Implementation()
{
	/** Technically the length of TargetActors should not be zero. */
	if (TargetActors.Num() == 0) return FRotator();

	FRotator OutputRotation;
	if (GroupRotationMethod == EGroupRotationMethod::ArithmeticAverage)
	{
		OutputRotation = GetAverageRotation(true);
	}
	else if (GroupRotationMethod == EGroupRotationMethod::WeighedAverage)
	{
		if (CheckWeightAllZero()) OutputRotation = GetAverageRotation(true);
		else OutputRotation = GetAverageRotation(false);
	}
	else if (GroupRotationMethod == EGroupRotationMethod::Specified)
	{
		if (SpecifiedRotationActor) OutputRotation = SpecifiedRotationActor->GetActorRotation();
		else OutputRotation = SpecifiedRotation;
	}
	else OutputRotation = FRotator();

	return OutputRotation;
}

FVector UECameraGroupActorComponent::GetAverageLocation(bool bIsArithemetic)
{	
	/** Get normalizer. */
	float Normalizer = 1.0f;
	if (!bIsArithemetic) Normalizer = GetNormalizer();

	/** Number of alive actors. */
	int NumberOfLiveActors = 0;
	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target)
			++NumberOfLiveActors;
	if (NumberOfLiveActors == 0) return FVector();

	/** Get average location. */
	FVector ResultLocation = FVector();
	for (int index = 0; index < TargetActors.Num(); ++index)
	{
		if (bIsArithemetic && TargetActors[index].Target) 
			ResultLocation += TargetActors[index].Target->GetActorLocation() / NumberOfLiveActors;
		else if (TargetActors[index].Target)
			ResultLocation += TargetActors[index].Target->GetActorLocation() * TargetActors[index].Weight / Normalizer;
	}

	return ResultLocation;
}

FRotator UECameraGroupActorComponent::GetAverageRotation(bool bIsArithemetic)
{
	/** Get normalizer. */
	float Normalizer = 1.0f;
	if (!bIsArithemetic) Normalizer = GetNormalizer();

	/** Number of alive actors. */
	int NumberOfLiveActors = 0;
	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target)
			++NumberOfLiveActors;
	if (NumberOfLiveActors == 0) return FRotator();

	/** Get average rotation. */
	FRotator ResultRotation = FRotator::ZeroRotator;
	float TotalWeight = 0.0f;

	for (int index = 0; index < TargetActors.Num(); ++index)
	{
		if (TargetActors[index].Target)
		{
			float CurrentWeight = bIsArithemetic ? 1.0f / NumberOfLiveActors : FMath::Square(TargetActors[index].Weight) / Normalizer;
			UpdateWeightedRotation(TotalWeight, ResultRotation, CurrentWeight, TargetActors[index].Target);
		}
	}

	return ResultRotation;
}

void UECameraGroupActorComponent::UpdateWeightedRotation(float& TotalWeight, FRotator& Rotation, float CurrentWeight, AActor* Camera)
{
	TotalWeight += CurrentWeight;

	// No accumulated weight yet, return.
	if (TotalWeight == 0.0f)
	{
		return;
	}

	float Alpha = CurrentWeight / TotalWeight;
	FRotator CameraRotation = Camera->GetActorRotation();

	// On the same side, just interp it!
	if ((Rotation.Yaw >= 0 && CameraRotation.Yaw >= 0) || (Rotation.Yaw <= 0 && CameraRotation.Yaw <= 0))
	{
		Rotation = (1 - Alpha) * Rotation + Alpha * CameraRotation;
	}

	// Otherwise, should check the angle difference.
	else
	{
		float AngleDifference = Rotation.Yaw - CameraRotation.Yaw;

		if (FMath::Abs(AngleDifference) > 180)
		{
			Rotation.Yaw >= 0 ? Rotation.Yaw -= 360 : CameraRotation.Yaw -= 360;
		}

		Rotation = (1 - Alpha) * Rotation + Alpha * CameraRotation;
	}
}

bool UECameraGroupActorComponent::CheckWeightAllZero()
{
	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target && TargetActors[index].Weight != 0)
			return false;
	return true;
}

float UECameraGroupActorComponent::GetNormalizer()
{
	float Normalizer = 0.0f;
	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target)
			Normalizer += TargetActors[index].Weight;
	return Normalizer;
}

FVector UECameraGroupActorComponent::GetFarestOrClosestLocation(bool bIsFarest)
{
	/** First find first alive actor and use its location. */
	int ResultIndex = -1;
	float ResultDistance = 0.0f;
	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target)
		{
			ResultIndex = index;
			ResultDistance = AnchorLocationActor ? FVector::DistSquared(AnchorLocationActor->GetActorLocation(), TargetActors[index].Target->GetActorLocation()) : FVector::DistSquared(AnchorLocation, TargetActors[index].Target->GetActorLocation());
			break;
		}

	/** All target actors are not alive. */
	if (ResultIndex == -1) return FVector();

	/** Else, find the closest or farest location. */
	for (int index = ResultIndex + 1; index < TargetActors.Num(); ++index)
	{
		float Distance = AnchorLocationActor ? FVector::DistSquared(AnchorLocationActor->GetActorLocation(), TargetActors[index].Target->GetActorLocation()) : FVector::DistSquared(AnchorLocation, TargetActors[index].Target->GetActorLocation());
		if ((!bIsFarest && Distance < ResultDistance) || (bIsFarest && Distance > ResultDistance))
		{
			ResultDistance = Distance;
			ResultIndex = index;
		}
	}

	return TargetActors[ResultIndex].Target->GetActorLocation();
}

FVector UECameraGroupActorComponent::GetLocationBasedOnDistance(bool bIsWeighted)
{
	/** AnchoredLocation is used for calculating distance. */
	FVector AnchoredLocation = AnchorLocationActor ? AnchorLocationActor->GetActorLocation() : AnchorLocation;
	FVector ResultLocation = FVector();
	float Normalizer = 0.0f;
	float WeightNormalizer = GetNormalizer();
	TArray<float> TempWeights;

	for (int index = 0; index < TargetActors.Num(); ++index)
	{
		if (TargetActors[index].Target)
		{
			/** Larger the distance is, smaller the weight is. */
			float InverseDistance = 100.0f / FVector::DistSquared(AnchoredLocation, TargetActors[index].Target->GetActorLocation());
			if (!bIsWeighted)
			{
				TempWeights.Add(InverseDistance);
				Normalizer += InverseDistance;
			}
			else
			{
				TempWeights.Add(InverseDistance * TargetActors[index].Weight / WeightNormalizer);
				Normalizer += InverseDistance * TargetActors[index].Weight / WeightNormalizer;
			}
		}
		else TempWeights.Add(-1.0f);
	}

	for (int index = 0; index < TargetActors.Num(); ++index)
		if (TargetActors[index].Target)
			ResultLocation += TargetActors[index].Target->GetActorLocation() * TempWeights[index] / Normalizer;
	
	return ResultLocation;
}