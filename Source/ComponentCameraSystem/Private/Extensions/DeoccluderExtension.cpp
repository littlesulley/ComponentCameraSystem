// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/DeoccluderExtension.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

UDeoccluderExtension::UDeoccluderExtension()
{
	/** Occluder is always applied after camera location and rotation have been determined. */
	Stage = EStage::PostAim;

	OccluderParams = FOccluderParams();
	MinimumDistanceFromTarget = 0.0f;
	CameraDistanceFromOcclusion = 0.0f;
	OcclusionDamping = 0.0f;
	RestoreDamping = 0.0f;

	OcclusionElapsedTime = 0.0f;
	CachedRawLocation = FVector(0, 0, 0);
	DeltaDistanceFromCamera = 0.0f;
	AlreadyDampedTime = 0.0f;
}

void UDeoccluderExtension::UpdateComponent_Implementation(float DeltaTime)
{
	/** Cache camera location at the very beginning. */
	CachedRawLocation = GetOwningActor()->GetActorLocation();

	/** Get the target to which we want to emit ray trace. */
	AActor* Target;
	if (GetOwningSettingComponent()->GetAimTarget() != nullptr)
	{
		Target = GetOwningSettingComponent()->GetAimTarget();
	}
	else if (GetOwningSettingComponent()->GetFollowTarget() != nullptr)
	{
		Target = GetOwningSettingComponent()->GetFollowTarget();
	}
	else return;

	/** Get ray start and end location. */
	FVector Direction = GetOwningActor()->GetActorLocation() - Target->GetActorLocation(); 
	Direction.Normalize();
	FVector Start = Target->GetActorLocation() + MinimumDistanceFromTarget * Direction;
	FVector End = GetOwningActor()->GetActorLocation();

	/** Do nothing if distance between camera and target is less than the required minimum distance.  */
	if (FVector::Distance(GetOwningActor()->GetActorLocation(), Target->GetActorLocation()) <= MinimumDistanceFromTarget)
	{
		ResetVariablesAndRestoreDamping(DeltaTime, DeltaDistanceFromCamera, RestoreDamping, Direction);
		return;
	}

	/** Find list of hits. */
	TArray<FHitResult> OutHits;

	TArray<AActor*> ActorsToIgnore;
	for (TSoftObjectPtr<AActor> SoftActor : OccluderParams.ActorsToIgnore)
	{
		if (SoftActor.IsValid())
		{
			ActorsToIgnore.Add(SoftActor.Get());
		}
	}

	if (OccluderParams.bTraceSingle)
	{
		FHitResult OutHit;
		bool bHit;

		if (OccluderParams.TraceShape == ETraceShape::Line)
		{
			bHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), Start, End, OccluderParams.ObjectTypes, false, ActorsToIgnore, OccluderParams.bShowDebug ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None, OutHit, true);
		}
		else if (OccluderParams.TraceShape == ETraceShape::Sphere)
		{
			bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), Start, End, OccluderParams.SphereRadius, OccluderParams.ObjectTypes, false, ActorsToIgnore, OccluderParams.bShowDebug ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None, OutHit, true);
		}
		else bHit = false;
		if (bHit) OutHits.Add(OutHit);
	}
	else
	{
		if (OccluderParams.TraceShape == ETraceShape::Line)
		{
			UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), Start, End, OccluderParams.ObjectTypes, false, ActorsToIgnore, OccluderParams.bShowDebug ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None, OutHits, true);
		}
		else if (OccluderParams.TraceShape == ETraceShape::Sphere)
		{
			UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, End, OccluderParams.SphereRadius, OccluderParams.ObjectTypes, false, ActorsToIgnore, OccluderParams.bShowDebug ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None, OutHits, true);
		}
	}

	/** Filter out actors with specific actor types. */
	for (TSubclassOf<AActor> ActorType : OccluderParams.ActorTypesToIgnore)
	{
		OutHits = OutHits.FilterByPredicate([ActorType](const FHitResult& HitResult) {
			return IsValid(HitResult.GetActor()) && !HitResult.GetActor()->IsA(ActorType);
		});
	}
	
	/** Resolve occlusion when result hits are not empty. */
	if (!(OutHits.Num() == 0))
	{
		/** Get the result hit used for relocating camera. 
		 *  If the first hit is inside a collider, we will use the next hit. 
		 *  Otherwise, it will use the first hit.
		 */
		FHitResult ResultHit;
		int32 ResultIndex;
		if (OutHits[0].bStartPenetrating)
		{
			if (OutHits.Num() >= 2) 
			{
				ResultHit = OutHits[1]; 
				ResultIndex = 1;
			}
			else
			{
				ResetVariablesAndRestoreDamping(DeltaTime, DeltaDistanceFromCamera, RestoreDamping, Direction);
				return;
			}
		}
		else 
		{
			ResultHit = OutHits[0]; 
			ResultIndex = 0;
		}

		/** Take MaximumTraceLength into account. If there is no valid hits, return without doing anything. */
		if (OccluderParams.MaximumTraceLength != 0.0f)
		{
			while (ResultIndex < OutHits.Num())
			{
				if (FVector::Distance(OutHits[ResultIndex].Location, GetOwningActor()->GetActorLocation()) <= OccluderParams.MaximumTraceLength)
					break;
				++ResultIndex;
			}
			if (ResultIndex < OutHits.Num()) ResultHit = OutHits[ResultIndex];
			else 
			{ 
				ResetVariablesAndRestoreDamping(DeltaTime, DeltaDistanceFromCamera, RestoreDamping, Direction);
				return;
			}
		}

		/** If occlusion has not lasted for MinimumOcclusionTime, do not take any action and wait. */
		OcclusionElapsedTime += DeltaTime;
		if (OcclusionElapsedTime < OccluderParams.MinimumOcclusionTime) return;
		
		/** All conditions have been satisfied. Begin resolving occlusion. */
		/** Get delta location. */
		FVector DeltaLocation = ResultHit.Location - GetOwningActor()->GetActorLocation();

		/** Take CameraDistanceFromOcclusion into account. */
		/** First find delta location. */
		DeltaLocation += CameraDistanceFromOcclusion * (-Direction);
		/** Then get delta distance (should exclude current DeltaDistanceFromCamera). */
		float DeltaDistance = DeltaLocation.Size() - DeltaDistanceFromCamera;

		/** Use OcclusionDamping to smoothly tranform camera to its desired location. */
		float DampedDeltaDistance;
		UECameraLibrary::DamperValue(FDampParams(EDampMethod::Naive, 0.00001f), DeltaTime, DeltaDistance, OcclusionDamping, DampedDeltaDistance);
		AlreadyDampedTime += DeltaTime;

		/** If already finishes damping, hard applies the delta distance. */
		if (AlreadyDampedTime >= OcclusionDamping)
		{
			DampedDeltaDistance = DeltaDistance;
		}

		/** Add camera location along the direction of sight. */
		DeltaDistanceFromCamera += DampedDeltaDistance;
		GetOwningActor()->AddActorWorldOffset(DeltaDistanceFromCamera * (-Direction));
	}
	/** If no occlusion is detected, reset OcclusionElapsedTime. */
	else
	{
		ResetVariablesAndRestoreDamping(DeltaTime, DeltaDistanceFromCamera, RestoreDamping, Direction);
		return;
	}
}

void UDeoccluderExtension::ResetVariablesAndRestoreDamping(float DeltaTime, const float& Input, float Damping, const FVector& Direction)
{
	ResetVariables();

	float DampedDeltaDistance;
	UECameraLibrary::DamperValue(FDampParams(EDampMethod::Naive, 0.00001f), DeltaTime, Input, Damping, DampedDeltaDistance);
	DeltaDistanceFromCamera = FMath::Max<float>(DeltaDistanceFromCamera - DampedDeltaDistance, 0.0f);

	GetOwningActor()->AddActorWorldOffset(DeltaDistanceFromCamera * (-Direction));
}

void UDeoccluderExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	CachedRawLocation = GetOwningActor()->GetActorLocation();
}

void UDeoccluderExtension::BindToOnPreTickComponent()
{
	if (GetOwningActor() != nullptr) GetOwningActor()->SetActorLocation(CachedRawLocation);
}