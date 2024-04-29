// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/ConfinerExtension.h"
#include "Utils/ECameraLibrary.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UConfinerExtension::UConfinerExtension()
{
	Stage = EStage::PostFollow;

	ExitDamping = 1.0f;
	EnterDamping = 0.2f;
	CachedRawLocation = FVector(0, 0, 0);
	CachedNewLocation = FVector(0, 0, 0);
}

void UConfinerExtension::UpdateComponent_Implementation(float DeltaTime)
{
	/** Cache camera location at the very beginning. */
	CachedRawLocation = GetOwningActor()->GetActorLocation();

	/** Check if current camera is within bounds. If true, reset damp progress. */
	bool bWithinBounds = CheckIfWithinBounds();

	FVector DesiredPosition;
	FVector DampedDeltaPosition;

	/** If raw camera position is within bounds, desired position will be the raw camera position. */
	if (bWithinBounds)
	{
		DesiredPosition = GetOwningActor()->GetActorLocation();
		DampedDeltaPosition = GetDampedDeltaPosition(DesiredPosition, DeltaTime, true);
	}
	/** Else, desired position is the nearest position on bounds to current camera position. */
	else
	{
		DesiredPosition = FindNearestPositionOnBounds();
		DampedDeltaPosition = GetDampedDeltaPosition(DesiredPosition, DeltaTime, false);
	}

	/** Set camera location and update cached new location. */
	GetOwningActor()->SetActorLocation(CachedNewLocation + DampedDeltaPosition);
	CachedNewLocation = GetOwningActor()->GetActorLocation();
}

bool UConfinerExtension::CheckIfWithinBounds()
{
	if (Boxes.Num() == 0) return true;

	for (TSoftObjectPtr<ATriggerBox> BoxPtr : Boxes)
	{
		ATriggerBox* Box = BoxPtr.Get();
		UBoxComponent* BoxCollisionComponent = CastChecked<UBoxComponent>(Box->GetCollisionComponent());
		bool bIsWithinBox = UKismetMathLibrary::IsPointInBox(GetOwningActor()->GetActorLocation(), Box->GetActorLocation(), BoxCollisionComponent->GetScaledBoxExtent());
		if (bIsWithinBox) return true;
	}

	return false;
}

FVector UConfinerExtension::FindNearestPositionOnBounds()
{ 
	FVector PositionOnBox;
	float ShortestDistance = 114514;
	int ResidingBox = 0;

	for (int index = 0; index < Boxes.Num(); ++index)
	{
		UBoxComponent* BoxCollisionComponent = CastChecked<UBoxComponent>(Boxes[index].Get()->GetCollisionComponent());
		
		FVector CurrentPositionOnBox;
		float Distance = BoxCollisionComponent->GetClosestPointOnCollision(GetOwningActor()->GetActorLocation(), CurrentPositionOnBox);

		if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			PositionOnBox = CurrentPositionOnBox;
			ResidingBox = index;
		}
	}
	
	return PositionOnBox;
}

FVector UConfinerExtension::GetDampedDeltaPosition(const FVector& DesiredPosition, float DeltaTime, bool bWithinBox)
{
	/** If not within box, blending to bounding position with Damping. */
	FVector DeltaPosition = DesiredPosition - CachedNewLocation;
	FVector DampedDeltaPosition;
	UECameraLibrary::DamperVectorWithSameDampTime(FDampParams(), DeltaTime, DeltaPosition, bWithinBox ? EnterDamping : ExitDamping, DampedDeltaPosition);
	return DampedDeltaPosition;
}

void UConfinerExtension::BindToOnPreTickComponent()
{
	if (GetOwningActor() != nullptr) GetOwningActor()->SetActorLocation(CachedRawLocation);
}
