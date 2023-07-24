// Copyright 2023 by Sulley. All Rights Reserved.

#include "Core/EPlayerCameraManager.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraManager.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "WaveOscillatorCameraShakePattern.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "Utils/ECameraShakeSourceActor.h"
#include "Utils/ECameraModifier_CameraShake.h"
#include "Utils/EPhotoCamera.h"
#include "Engine/Scene.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraTypes.h"
#include "Camera/CameraShakeSourceActor.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "UObject/ScriptInterface.h"
#include "Engine/BlendableInterface.h"
#include "Components/MeshComponent.h"

AEPlayerCameraManager::AEPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	/** Use our customized camera shake modifier class. */
	DefaultModifiers.Empty();
	DefaultModifiers.Add(UECameraModifier_CameraShake::StaticClass());

	bEnableDither = true;
	DitherFadeParams = FDitherParams();
	MaxWeight = 1.0f;
	DitherFadeInTime = 1.0f;
	DitherFadeOutTime = 1.0f;
	DitherTickInterval = 0.1f;
	ElapsedIntervalTime = 0.0f;
}

void AEPlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	Super::ApplyCameraModifiers(DeltaTime, InOutPOV);

	/** For post process blendables. */
	check(PostProcessMaterialSettings.WeightedBlendables.Array.Num() == WeightedBlendables.Num());
	
	for (int i = 0; i < WeightedBlendables.Num(); ++i)
	{
		FWeightedBlendableObject& WeightedBB = WeightedBlendables[i];
		float Weight = GetPPWeight(WeightedBB.Weight, WeightedBB.BlendInTime, WeightedBB.Duration, WeightedBB.BlendOutTime, WeightedBB.ElapsedBlendInTime, WeightedBB.ElapsedDurationTime, WeightedBB.ElapsedBlendOutTime, WeightedBB.bHasCompleted, DeltaTime);
		if (!WeightedBB.bHasCompleted)
		{
			/** The per-material weight only works in a very weird way requiring specific material setups.
			 *  See https://forums.unrealengine.com/t/post-process-blend-weight-acts-as-boolean-no-fade-in-out/368921/6.
			 */
			PostProcessMaterialSettings.WeightedBlendables.Array[i].Weight = Weight;
		}
		else
		{
			PostProcessMaterialSettings.WeightedBlendables.Array[i].Weight = 0.0f;
		}
	}

	AddCachedPPBlend(PostProcessMaterialSettings, MaterialWeight);

	/** For post process settings. */
	for (FWeightedPostProcess& WeightedPP : WeightedPostProcesses)
	{
		float Weight = GetPPWeight(WeightedPP.Weight, WeightedPP.BlendInTime, WeightedPP.Duration, WeightedPP.BlendOutTime, WeightedPP.ElapsedBlendInTime, WeightedPP.ElapsedDurationTime, WeightedPP.ElapsedBlendOutTime, WeightedPP.bHasCompleted, DeltaTime);
		if (!WeightedPP.bHasCompleted)
		{
			AddCachedPPBlend(WeightedPP.PPSetting, Weight);
		}
	}
}

float AEPlayerCameraManager::GetPPWeight(float& Weight, float& BlendInTime, float& Duration, float& BlendOutTime, float& ElapsedBlendInTime, float& ElapsedDurationTime, float& ElapsedBlendOutTime, bool& bHasCompleted, const float& DeltaTime)
{
	/** If blending in. */
	if (ElapsedBlendInTime < BlendInTime)
	{
		ElapsedBlendInTime = FMath::Clamp<float>(ElapsedBlendInTime + DeltaTime, 0, BlendInTime);
		float ResultWeight = GetBlendedWeight(0.0f, Weight, BlendInTime, ElapsedBlendInTime);
		return ResultWeight;
	}

	/** Has blended in and duration is zero (infinite). */
	else if (Duration == 0.0f)
	{
		return Weight;
	}

	/** Has blended in and duration is not zero.  */
	else if (Duration != 0.0f && ElapsedDurationTime < Duration)
	{
		ElapsedDurationTime += DeltaTime;
		return Weight;
	}

	/** If blending out. */
	else if (ElapsedBlendOutTime < BlendOutTime)
	{
		ElapsedBlendOutTime = FMath::Clamp<float>(ElapsedBlendOutTime + DeltaTime, 0, BlendOutTime);
		float ResultWeight = GetBlendedWeight(Weight, 0.0f, BlendOutTime, ElapsedBlendOutTime);
		return ResultWeight;
	}
	/** Has finished blending out. Set bHasCompleted to true. */
	else
	{
		bHasCompleted = true;
		return 0.0f;
	}
}

void AEPlayerCameraManager::DoUpdateCamera(float DeltaTime)
{
	Super::DoUpdateCamera(DeltaTime);

	if (bEFading)
	{
		EFadeTimeRemaining = FMath::Max(EFadeTimeRemaining - DeltaTime, 0.0f);
		/** Fading in. */
		if (EFadeTimeRemaining >= EFadeDuration + EFadeOutTime)
		{
			FadeAmount = UKismetMathLibrary::Ease(EFadeAlpha.X, EFadeAlpha.Y, (EFadeInTime + EFadeDuration + EFadeOutTime - EFadeTimeRemaining) / EFadeInTime, EFadeInFunc);
		}
		/** Maintaining. */
		else if (EFadeTimeRemaining >= EFadeOutTime)
		{
			FadeAmount = EFadeAlpha.Y;
		}
		/** Fading out. */
		else
		{
			FadeAmount = UKismetMathLibrary::Ease(EFadeAlpha.Y, EFadeAlpha.X, 1.0f - EFadeTimeRemaining / EFadeOutTime, EFadeOutFunc);
		}
	}

	if (bEnableDither)
	{
		DoDitherFade(DeltaTime);
	}
}

void AEPlayerCameraManager::DoDitherFade(float DeltaTime)
{
	ElapsedIntervalTime += DeltaTime;

	/** Get the target to which we want to emit ray trace. */
	AActor* Target = nullptr;
	if (IsValid(UECameraLibrary::GetActiveCamera(GetWorld())) && IsValid(UECameraLibrary::GetActiveCamera(GetWorld())->GetSettingsComponent()->GetAimTarget()))
	{
		Target = UECameraLibrary::GetActiveCamera(GetWorld())->GetSettingsComponent()->GetAimTarget();
	}
	else if (IsValid(UECameraLibrary::GetActiveCamera(GetWorld())) && IsValid(UECameraLibrary::GetActiveCamera(GetWorld())->GetSettingsComponent()->GetFollowTarget()))
	{
		Target = UECameraLibrary::GetActiveCamera(GetWorld())->GetSettingsComponent()->GetFollowTarget();
	}

	/** Do self dither fade. */
	if (DitherFadeParams.bEnableSelfDither)
	{
		if (!IsValid(GetOwningPlayerController()->GetPawn()))
		{
			DitherFadeActors.Empty();
		}
		else
		{
			if (ElapsedIntervalTime >= DitherTickInterval)
			{
				TArray<FHitResult> OutHits = DoDitherCollisionCheck(GetOwningPlayerController()->GetPawn(), DitherFadeParams.DitherSelfTraceLength, DitherFadeParams.SelfType);
				UpdateSelfDitherWithNew(DeltaTime, OutHits);
			}
			else
			{
				UpdateSelfDitherWithDefault(DeltaTime);
			}
		}

		UpdateDitherWeight(DitherFadeSelf);
	}

	/** Do dither fade for other actors. */
	if (!IsValid(Target))
	{
		UpdateDitherWithFadeOut(DeltaTime);
	}
	else
	{
		if (ElapsedIntervalTime >= DitherTickInterval)
		{
			TArray<FHitResult> OutHits = DoDitherCollisionCheck(Target, DitherFadeParams.MaxTraceLength, DitherFadeParams.ObjectTypes);
			UpdateDitherWithNew(DeltaTime, OutHits);
		}
		else
		{
			UpdateDitherWithDefault(DeltaTime);
		}
	}

	if (ElapsedIntervalTime >= DitherTickInterval)
	{
		ElapsedIntervalTime = 0.0f;
	}
	
	UpdateDitherWeight(DitherFadeActors);
}

TArray<FHitResult> AEPlayerCameraManager::DoDitherCollisionCheck(AActor* Target, float MaxLength, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes)
{
	FVector Start = GetCameraLocation();
	FVector End = Target->GetActorLocation();

	if (MaxLength != 0.0f)
	{
		FVector Direction = End - Start;
		Direction.Normalize();
		End = Start + MaxLength * Direction;
	}

	/** Get all out hits and filter. */
	TArray<FHitResult> OutHits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Start, End, DitherFadeParams.SphereRadius, ObjectTypes, false, DitherFadeParams.ActorsToIgnore, DitherFadeParams.bShowDebug ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None, OutHits, true);
	for (TSubclassOf<AActor> ActorType : DitherFadeParams.ActorTypesToIgnore)
	{
		OutHits = OutHits.FilterByPredicate([ActorType](const FHitResult& HitResult) {
			return IsValid(HitResult.GetActor()) && !HitResult.GetActor()->IsA(ActorType);
		});
	}

	return OutHits;
}

void AEPlayerCameraManager::UpdateDitherWithFadeOut(float DeltaTime)
{
	TArray<const AActor*> ActorsToRemove;

	for (TPair<const AActor*, FFadedObjectInfo>& Pair : DitherFadeActors)
	{
		if (!IsValid(Pair.Key))
		{
			ActorsToRemove.Add(Pair.Key);
			continue;
		}

		if (Pair.Value.bFading)
		{
			Pair.Value.bFading = false;
			Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + (1.0f - Pair.Value.Weight) * DitherFadeOutTime, 0.0f, DitherFadeOutTime);
			Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
		}
		else
		{
			if (Pair.Value.ElapsedFadeOutTime >= DitherFadeOutTime)
			{
				ActorsToRemove.Add(Pair.Key);
			}
			else
			{
				Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeOutTime, 0.0f, DitherFadeOutTime);
				Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
			}
		}
	}

	RestoreDitherMaterials(ActorsToRemove);
}

void AEPlayerCameraManager::UpdateSelfDitherWithDefault(float DeltaTime)
{
	TArray<const APawn*> PawnsToRemove;

	for (TPair<const APawn*, FFadedObjectInfo>& Pair : DitherFadeSelf)
	{
		if (!IsValid(Pair.Key))
		{
			PawnsToRemove.Add(Pair.Key);
			continue;
		}

		float TargetWeight = FMath::Clamp((DitherFadeParams.DitherSelfTraceLength - Pair.Value.Distance) / (DitherFadeParams.DitherSelfTraceLength - DitherFadeParams.DitherSelfMaxWeightDistance), 0.0f, 1.0f);
		
		if (Pair.Value.bFading)
		{
			Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeInTime, 0.0f, DitherFadeInTime);
			Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
		}
		else
		{
			if (Pair.Value.ElapsedFadeOutTime >= DitherFadeOutTime)
			{
				PawnsToRemove.Add(Pair.Key);
			}
			else
			{
				Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeOutTime, 0.0f, DitherFadeOutTime);
				Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
			}
		}

		Pair.Value.Weight = FMath::Clamp(Pair.Value.Weight, 0.0f, TargetWeight);
	}

	RestoreDitherMaterials(PawnsToRemove);
}

void AEPlayerCameraManager::UpdateDitherWithDefault(float DeltaTime)
{
	TArray<const AActor*> ActorsToRemove;

	for (TPair<const AActor*, FFadedObjectInfo>& Pair : DitherFadeActors)
	{
		if (!IsValid(Pair.Key))
		{
			ActorsToRemove.Add(Pair.Key);
			continue;
		}

		if (Pair.Value.bFading)
		{
			Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeInTime, 0.0f, DitherFadeInTime);
			Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
		}
		else
		{
			if (Pair.Value.ElapsedFadeOutTime >= DitherFadeOutTime)
			{
				ActorsToRemove.Add(Pair.Key);
			}
			else
			{
				Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeOutTime, 0.0f, DitherFadeOutTime);
				Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
			}
		}
	}

	RestoreDitherMaterials(ActorsToRemove);
}

void AEPlayerCameraManager::UpdateSelfDitherWithNew(float DeltaTime, TArray<FHitResult>& OutHits)
{
	TSet<APawn*> CurrentFadingPawns;

	for (const FHitResult& HitResult : OutHits)
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (IsValid(HitPawn))
		{
			CurrentFadingPawns.Add(HitPawn);
			FFadedObjectInfo* PawnInfo = DitherFadeSelf.Find(HitPawn);
			if (PawnInfo == nullptr)
			{
				OverrideDitherMaterials(HitPawn);
			}
			PawnInfo = DitherFadeSelf.Find(HitPawn);
			PawnInfo->Distance = HitResult.Distance;
		}
	}

	TArray<const APawn*> PawnsToRemove;

	for (TPair<const APawn*, FFadedObjectInfo>& Pair : DitherFadeSelf)
	{
		if (!IsValid(Pair.Key))
		{
			PawnsToRemove.Add(Pair.Key);
			continue;
		}

		if (!CurrentFadingPawns.Contains(Pair.Key))
		{
			if (Pair.Value.bFading)
			{
				Pair.Value.bFading = false;
				Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + (1.0f - Pair.Value.Weight) * DitherFadeOutTime, 0.0f, DitherFadeOutTime);
				Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
			}
			else
			{
				if (Pair.Value.ElapsedFadeOutTime >= DitherFadeOutTime)
				{
					PawnsToRemove.Add(Pair.Key);
				}
				else
				{
					Pair.Value.ElapsedFadeOutTime = FMath::Clamp(Pair.Value.ElapsedFadeOutTime + DeltaTime, 0.0f, DitherFadeOutTime);
					Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
				}
			}
		}
		else
		{
			float TargetWeight = FMath::Clamp((DitherFadeParams.DitherSelfTraceLength - Pair.Value.Distance) / (DitherFadeParams.DitherSelfTraceLength - DitherFadeParams.DitherSelfMaxWeightDistance), 0.0f, 1.0f);

			if (Pair.Value.bFading)
			{
				Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeInTime, 0.0f, DitherFadeInTime);
				Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
			}
			else
			{
				Pair.Value.bFading = true;
				Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.Weight * DitherFadeInTime, 0.0f, DitherFadeInTime);
				Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
			}

			Pair.Value.Weight = FMath::Clamp(Pair.Value.Weight, 0.0f, TargetWeight);
		}
	}

	RestoreDitherMaterials(PawnsToRemove);
}

void AEPlayerCameraManager::UpdateDitherWithNew(float DeltaTime, TArray<FHitResult>& OutHits)
{
	TSet<AActor*> CurrentFadingActors;

	for (const FHitResult& HitResult : OutHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor))
		{
			CurrentFadingActors.Add(HitActor);
			FFadedObjectInfo* ActorInfo = DitherFadeActors.Find(HitActor);
			if (ActorInfo == nullptr)
			{
				OverrideDitherMaterials(HitActor);
			}
		}
	}

	TArray<const AActor*> ActorsToRemove;

	for (TPair<const AActor*, FFadedObjectInfo>& Pair : DitherFadeActors)
	{
		if (!IsValid(Pair.Key))
		{
			ActorsToRemove.Add(Pair.Key);
			continue;
		}

		/** Not a current fading actor, should check if it's already fading out. */
		if (!CurrentFadingActors.Contains(Pair.Key))
		{
			if (Pair.Value.bFading)
			{
				Pair.Value.bFading = false;
				Pair.Value.ElapsedFadeOutTime = FMath::Clamp(DeltaTime + (1.0f - Pair.Value.Weight) * DitherFadeOutTime, 0.0f, DitherFadeOutTime);
				Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
			}
			else
			{
				if (Pair.Value.ElapsedFadeOutTime >= DitherFadeOutTime)
				{
					ActorsToRemove.Add(Pair.Key);
				}
				else
				{
					Pair.Value.ElapsedFadeOutTime = FMath::Clamp(Pair.Value.ElapsedFadeOutTime + DeltaTime, 0.0f, DitherFadeOutTime);
					Pair.Value.Weight = 1.0f - Pair.Value.ElapsedFadeOutTime / DitherFadeOutTime;
				}
			}
		}
		else
		{
			if (Pair.Value.bFading)
			{
				Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.ElapsedFadeInTime, 0.0f, DitherFadeInTime);
				Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
			}
			else
			{
				Pair.Value.bFading = true;
				Pair.Value.ElapsedFadeInTime = FMath::Clamp(DeltaTime + Pair.Value.Weight * DitherFadeInTime, 0.0f, DitherFadeInTime);
				Pair.Value.Weight = Pair.Value.ElapsedFadeInTime / DitherFadeInTime;
			}
		}
	}

	RestoreDitherMaterials(ActorsToRemove);
}

void AEPlayerCameraManager::UpdateDitherWeightSingle(const AActor* Actor, FFadedObjectInfo& FadeInfo)
{
	/** For all meshes. */
	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		/** For all materials. */
		TArray<UMaterialInterface*> MaterialInterfaces = Mesh->GetMaterials();
		for (UMaterialInterface* MaterialInterface : MaterialInterfaces)
		{
			if (IsValid(MaterialInterface))
			{
				/** Find all scalar parameters within this material. We need to set the scalar value named `EFadeWeight`. */
				TArray<FMaterialParameterInfo> MaterialScalarParameters;
				TArray<FGuid> MaterialScalarParameterIDs;
				MaterialInterface->GetAllScalarParameterInfo(MaterialScalarParameters, MaterialScalarParameterIDs);

				for (FMaterialParameterInfo& MaterialParameterInfo : MaterialScalarParameters)
				{
					if (MaterialParameterInfo.Name.IsEqual(FName("EFadeWeight")))
					{
						float FadeWeight = FMath::Clamp(FadeInfo.Weight, 0.0f, MaxWeight);
						Mesh->SetScalarParameterValueOnMaterials(MaterialParameterInfo.Name, FadeWeight);
					}
				}
			}
		}
	}
}

void AEPlayerCameraManager::UpdateDitherWeight(TMap<const AActor*, FFadedObjectInfo> FadeActors)
{
	for (TPair<const AActor*, FFadedObjectInfo>& Pair : FadeActors)
	{
		if (!IsValid(Pair.Key)) continue;
		UpdateDitherWeightSingle(Pair.Key, Pair.Value);
	}
}

void AEPlayerCameraManager::UpdateDitherWeight(TMap<const APawn*, FFadedObjectInfo> FadeSelf)
{
	for (TPair<const APawn*, FFadedObjectInfo>& Pair : FadeSelf)
	{
		if (!IsValid(Pair.Key)) continue;
		UpdateDitherWeightSingle(Pair.Key, Pair.Value);
	}
}

void AEPlayerCameraManager::OverrideDitherMaterials(const AActor* Actor)
{
	FFadedObjectInfo ObjectInfo;
	DitherFadeActors.Add(Actor, ObjectInfo);
}

void AEPlayerCameraManager::OverrideDitherMaterials(const APawn* Pawn)
{
	FFadedObjectInfo ObjectInfo;
	DitherFadeSelf.Add(Pawn, ObjectInfo);
}

void AEPlayerCameraManager::RestoreDitherMaterials(TArray<const AActor*> Actors)
{
	for (const AActor* Actor : Actors)
	{
		DitherFadeActors.Remove(Actor);
	}
}

void AEPlayerCameraManager::RestoreDitherMaterials(TArray<const APawn*> Pawns)
{
	for (const APawn* Pawn : Pawns)
	{
		DitherFadeSelf.Remove(Pawn);
	}
}

void AEPlayerCameraManager::AddBlendable(const TScriptInterface<IBlendableInterface>& InBlendableObject, const float InWeight, const float InBlendInTime, const float InDuration, const float InBlendOutTime)
{
	RemoveCompletedBlendables();

	FWeightedBlendableObject NewWeightedBlendable = FWeightedBlendableObject(InBlendableObject, InWeight, InBlendInTime, InDuration, InBlendOutTime);
	PostProcessMaterialSettings.AddBlendable(InBlendableObject, 0.0f);
	WeightedBlendables.Add(NewWeightedBlendable);
}

void AEPlayerCameraManager::RemoveBlendable(const TScriptInterface<IBlendableInterface>& InBlendableObject)
{
	PostProcessMaterialSettings.RemoveBlendable(InBlendableObject);

	for (int i = 0, count = WeightedBlendables.Num(); i < count; ++i)
	{
		FWeightedBlendableObject& WeightedBlendable = WeightedBlendables[i];
		if (WeightedBlendable.InBlendableObject.GetObject() == InBlendableObject.GetObject())
		{
			WeightedBlendables.RemoveAt(i);
			break;
		}
	}
}

void AEPlayerCameraManager::AddPostProcess(const FPostProcessSettings& InPostProcess, const float InWeight, const float InBlendInTime, const float InDuration, const float InBlendOutTime)
{
	RemoveCompletedPostProcesses();

	FWeightedPostProcess NewWeightedPP = FWeightedPostProcess(InPostProcess, InWeight, InBlendInTime, InDuration, InBlendOutTime);
	WeightedPostProcesses.Add(NewWeightedPP);
}

void AEPlayerCameraManager::RemoveCompletedPostProcesses()
{
	for (int i = 0, count = WeightedPostProcesses.Num(); i < count; ++i)
	{
		FWeightedPostProcess& WeightedPP = WeightedPostProcesses[i];
		if (WeightedPP.bHasCompleted)
		{
			WeightedPostProcesses.RemoveAt(i);
			--i;
			--count;
		}
	}
}

void AEPlayerCameraManager::RemoveCompletedBlendables()
{
	for (int i = 0, count = WeightedBlendables.Num(); i < count; ++i)
	{
		FWeightedBlendableObject& WeightedBlendable = WeightedBlendables[i];
		if (WeightedBlendable.bHasCompleted)
		{
			PostProcessMaterialSettings.RemoveBlendable(WeightedBlendable.InBlendableObject);
			WeightedBlendables.RemoveAt(i);
			--i;
			--count;
		}
	}
}

void AEPlayerCameraManager::RemoveAllPostProcesses()
{
	WeightedPostProcesses.Empty();
}

void AEPlayerCameraManager::EasyStartCameraFade(float FromAlpha, float ToAlpha, float FadeInTime, TEnumAsByte<EEasingFunc::Type> FadeInFunc, float Duration, float FadeOutTime, TEnumAsByte<EEasingFunc::Type> FadeOutFunc, FLinearColor Color)
{
	bEnableFading = true;
	bEFading = true;

	EFadeAlpha = FVector2D(FromAlpha, ToAlpha);
	EFadeInTime = FadeInTime;
	EFadeOutTime = FadeOutTime;
	EFadeDuration = FadeOutTime;
	EFadeTimeRemaining = FadeInTime + Duration + FadeOutTime;
	EFadeInFunc = FadeInFunc;
	EFadeOutFunc = FadeOutFunc;
	FadeColor = Color;

	/** Should sync FadeTimeRemaining in case of immediately stopping camera fade when calling the parent update function. */
	FadeTimeRemaining = EFadeTimeRemaining;
}

void AEPlayerCameraManager::EasyStopCameraFade(float StopAlpha)
{
	if (bEnableFading && bEFading)
	{
		FadeAmount = StopAlpha;
		bEnableFading = false;
		bEFading = false;
	}
}

void AEPlayerCameraManager::ImmediatelyClearDither()
{
	for (TPair<const APawn*, FFadedObjectInfo>& Pair : DitherFadeSelf)
	{
		if (IsValid(Pair.Key))
		{
			Pair.Value.Weight = 0.0f;
		}
	}

	for (TPair<const AActor*, FFadedObjectInfo>& Pair : DitherFadeActors)
	{
		if (IsValid(Pair.Key))
		{
			Pair.Value.Weight = 0.0f;
		}
	}

	UpdateDitherWeight(DitherFadeSelf);
	UpdateDitherWeight(DitherFadeActors);

	DitherFadeSelf.Empty();
	DitherFadeActors.Empty();
}

UCameraShakeBase* AEPlayerCameraManager::StartCameraShakeWave(float Scale, 
															  ECameraShakePlaySpace PlaySpace, 
															  FRotator UserPlaySpaceRot,
															  FWaveOscillator X,
															  FWaveOscillator Y,
															  FWaveOscillator Z,
															  FWaveOscillator Pitch,
															  FWaveOscillator Yaw,
															  FWaveOscillator Roll,
															  FWaveOscillator FOV,
															  bool bSingleInstance,
															  UCameraShakeSourceComponent* CameraShakeSourceComponent,
															  float LocationAmplitudeMultiplier, 
															  float LocationFrequencyMultiplier,
															  float RotationAmplitudeMultiplier, 
															  float RotationFrequencyMultiplier,
															  float Duration, 
															  float BlendInTime, 
															  float BlendOutTime)
{
	if (CachedCameraShakeMod && (Scale > 0.0f))
	{
		UWaveOscillatorCameraShakePattern* WavePattern = NewObject<UWaveOscillatorCameraShakePattern>();
		WavePattern->LocationAmplitudeMultiplier = LocationAmplitudeMultiplier;
		WavePattern->LocationFrequencyMultiplier = LocationFrequencyMultiplier;
		WavePattern->X = X;
		WavePattern->Y = Y;
		WavePattern->Z = Z;
		WavePattern->RotationAmplitudeMultiplier = RotationAmplitudeMultiplier;
		WavePattern->RotationFrequencyMultiplier = RotationFrequencyMultiplier;
		WavePattern->Pitch = Pitch;
		WavePattern->Yaw = Yaw;
		WavePattern->Roll = Roll;
		WavePattern->FOV = FOV;
		WavePattern->Duration = Duration;
		WavePattern->BlendInTime = BlendInTime;
		WavePattern->BlendOutTime = BlendOutTime;

		UCameraShakeBase* CameraShake = Cast<UECameraModifier_CameraShake>(CachedCameraShakeMod)->AddCameraShakeWave(WavePattern, FAddCameraShakeParams(Scale, PlaySpace, UserPlaySpaceRot, CameraShakeSourceComponent), bSingleInstance);
		return CameraShake;
	}

	return nullptr;
}

UCameraShakeBase* AEPlayerCameraManager::StartCameraShakePerlin(float Scale,
																ECameraShakePlaySpace PlaySpace,
																FRotator UserPlaySpaceRot,
																FPerlinNoiseShaker X,
																FPerlinNoiseShaker Y,
																FPerlinNoiseShaker Z,
																FPerlinNoiseShaker Pitch,
																FPerlinNoiseShaker Yaw,
																FPerlinNoiseShaker Roll,
																FPerlinNoiseShaker FOV,
																bool bSingleInstance,
																UCameraShakeSourceComponent* CameraShakeSourceComponent,
																float LocationAmplitudeMultiplier,
																float LocationFrequencyMultiplier,
																float RotationAmplitudeMultiplier,
																float RotationFrequencyMultiplier,
																float Duration,
																float BlendInTime,
																float BlendOutTime)
{
	if (CachedCameraShakeMod && (Scale > 0.0f))
	{
		UPerlinNoiseCameraShakePattern* PerlinPattern = NewObject<UPerlinNoiseCameraShakePattern>();
		PerlinPattern->LocationAmplitudeMultiplier = LocationAmplitudeMultiplier;
		PerlinPattern->LocationFrequencyMultiplier = LocationFrequencyMultiplier;
		PerlinPattern->X = X;
		PerlinPattern->Y = Y;
		PerlinPattern->Z = Z;
		PerlinPattern->RotationAmplitudeMultiplier = RotationAmplitudeMultiplier;
		PerlinPattern->RotationFrequencyMultiplier = RotationFrequencyMultiplier;
		PerlinPattern->Pitch = Pitch;
		PerlinPattern->Yaw = Yaw;
		PerlinPattern->Roll = Roll;
		PerlinPattern->FOV = FOV;
		PerlinPattern->Duration = Duration;
		PerlinPattern->BlendInTime = BlendInTime;
		PerlinPattern->BlendOutTime = BlendOutTime;

		UCameraShakeBase* CameraShake = Cast<UECameraModifier_CameraShake>(CachedCameraShakeMod)->AddCameraShakePerlin(PerlinPattern, FAddCameraShakeParams(Scale, PlaySpace, UserPlaySpaceRot, CameraShakeSourceComponent), bSingleInstance);
		return CameraShake;
	}

	return nullptr;
}

UCameraShakeBase* AEPlayerCameraManager::EasyStartCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, 
															  FPackedOscillationParams ShakeParams, 
															  float Scale, 
															  ECameraShakePlaySpace PlaySpace, 
															  FRotator UserPlaySpaceRot, 
															  bool bSingleInstance,
															  UCameraShakeSourceComponent* CameraShakeSourceComponent)
{
	if (CachedCameraShakeMod && (Scale > 0.0f))
	{
		if (ShakeClass != nullptr)
		{
			return CachedCameraShakeMod->AddCameraShake(ShakeClass, FAddCameraShakeParams(Scale, PlaySpace, UserPlaySpaceRot, CameraShakeSourceComponent));
		}

		if (ShakeParams.ShakeType == ECameraShakeType::Wave)
		{
			return StartCameraShakeWave(Scale, 
										PlaySpace, 
										UserPlaySpaceRot, 
										ShakeParams.X,
										ShakeParams.Y,
										ShakeParams.Z,
										ShakeParams.Pitch,
										ShakeParams.Yaw,
										ShakeParams.Roll,
										ShakeParams.FOV,
										bSingleInstance, 
										CameraShakeSourceComponent,
										ShakeParams.LocationAmplitudeMultiplier, 
										ShakeParams.LocationFrequencyMultiplier,
										ShakeParams.RotationAmplitudeMultiplier, 
										ShakeParams.RotationFrequencyMultiplier,
										ShakeParams.Duration, 
										ShakeParams.BlendInTime, 
										ShakeParams.BlendOutTime
			);
		}
		else
		{
			FPerlinNoiseShaker X     = FPerlinNoiseShaker();
			FPerlinNoiseShaker Y     = FPerlinNoiseShaker();
			FPerlinNoiseShaker Z     = FPerlinNoiseShaker();
			FPerlinNoiseShaker Pitch = FPerlinNoiseShaker();
			FPerlinNoiseShaker Yaw   = FPerlinNoiseShaker();
			FPerlinNoiseShaker Roll  = FPerlinNoiseShaker();
			FPerlinNoiseShaker FOV   = FPerlinNoiseShaker();

			X.Amplitude = ShakeParams.X.Amplitude,          X.Frequency = ShakeParams.X.Frequency;
			Y.Amplitude = ShakeParams.Y.Amplitude,          Y.Frequency = ShakeParams.Y.Frequency;
			Z.Amplitude = ShakeParams.Z.Amplitude,          Z.Frequency = ShakeParams.Z.Frequency;
			Pitch.Amplitude = ShakeParams.Pitch.Amplitude,  Pitch.Frequency = ShakeParams.Pitch.Frequency;
			Yaw.Amplitude = ShakeParams.Yaw.Amplitude,      Yaw.Frequency = ShakeParams.Yaw.Frequency;
			Roll.Amplitude = ShakeParams.Roll.Amplitude,    Roll.Frequency = ShakeParams.Roll.Frequency;
			FOV.Amplitude = ShakeParams.FOV.Amplitude,      FOV.Frequency = ShakeParams.FOV.Frequency;

			return StartCameraShakePerlin(Scale,
										PlaySpace,
										UserPlaySpaceRot,
										X,
										Y,
										Z,
										Pitch,
										Yaw,
										Roll,
										FOV,
										bSingleInstance,
										CameraShakeSourceComponent,
										ShakeParams.LocationAmplitudeMultiplier,
										ShakeParams.LocationFrequencyMultiplier,
										ShakeParams.RotationAmplitudeMultiplier,
										ShakeParams.RotationFrequencyMultiplier,
										ShakeParams.Duration,
										ShakeParams.BlendInTime,
										ShakeParams.BlendOutTime
			);
		}
	}

	return nullptr;
}

UCameraShakeBase* AEPlayerCameraManager::EasyStartCameraShakeFromSource(TSubclassOf<UCameraShakeBase> ShakeClass,
																		FPackedOscillationParams ShakeParams,
																		AActor* SpawnActor,
																		FVector SpawnLocation,
																		ECameraShakeAttenuation Attenuation,
																		float InnerAttenuationRadius,
																		float OuterAttenuationRadius,
																		float Scale,
																		ECameraShakePlaySpace PlaySpace,
																		FRotator UserPlaySpaceRot,
																		bool bSingleInstance)
{
	if (CachedCameraShakeMod && (Scale > 0.0f))
	{
		AECameraShakeSourceActor* CameraSourceActor = nullptr;

		if (SpawnActor != nullptr)
		{
			bool bHasShakeSourceActor = false;

			/** Check if already has a ECameraShakeSourceActor actor attached to SpawnActor. */
			TArray<AActor*> OutActors;
			SpawnActor->GetAttachedActors(OutActors);

			for (AActor* ChildActor : OutActors)
			{
				if (ChildActor->IsA<AECameraShakeSourceActor>())
				{
					bHasShakeSourceActor = true;
					CameraSourceActor = Cast<AECameraShakeSourceActor>(ChildActor);
					break;
				}
			}

			/** If not, instantiate a new one. */
			if (!bHasShakeSourceActor)
			{
				CameraSourceActor = GetWorld()->SpawnActor<AECameraShakeSourceActor>();
				CameraSourceActor->SetActorTransform(FTransform());
				CameraSourceActor->AttachToActor(SpawnActor, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
			}
		}
		else
		{
			bool bHasShakeSourceActor = false;

			/** Check if already has a ECameraShakeSourceActor actor in level. */
			TArray<AActor*> OutActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AECameraShakeSourceActor::StaticClass(), OutActors);

			if (OutActors.Num() == 0)
			{
				CameraSourceActor = GetWorld()->SpawnActor<AECameraShakeSourceActor>(SpawnLocation, FRotator());
			}
			else
			{
				CameraSourceActor = (AECameraShakeSourceActor*)OutActors[0];
				CameraSourceActor->SetActorLocation(SpawnLocation);
			}
		}

		if (CameraSourceActor != nullptr)
		{
			CameraSourceActor->GetCameraShakeSourceComponent()->Attenuation = Attenuation;
			CameraSourceActor->GetCameraShakeSourceComponent()->InnerAttenuationRadius = InnerAttenuationRadius;
			CameraSourceActor->GetCameraShakeSourceComponent()->OuterAttenuationRadius = OuterAttenuationRadius;
		}

		return EasyStartCameraShake(ShakeClass, ShakeParams, Scale, PlaySpace, UserPlaySpaceRot, bSingleInstance, CameraSourceActor->GetCameraShakeSourceComponent());
	}

	return nullptr;
}

float AEPlayerCameraManager::GetBlendedWeight(const float& StartWeight, const float& TargetWeight, const float& BlendTime, const float& ElapsedTime)
{
	float ResultWeight = StartWeight + (TargetWeight - StartWeight) * ElapsedTime / BlendTime;
	if (ResultWeight < 1e-5) ResultWeight = 0.f;
	return ResultWeight;
}

void AEPlayerCameraManager::SwitchPhotoMode()
{
	if (!IsValid(PhotoCamera))
	{
		AActor* PhotoCameraActor = UGameplayStatics::GetActorOfClass(this, AEPhotoCamera::StaticClass());

		if (IsValid(PhotoCameraActor))
		{
			PhotoCamera = Cast<AEPhotoCamera>(PhotoCameraActor);
		}
		else
		{
			PhotoCamera = GetWorld()->SpawnActor<AEPhotoCamera>();
		}
	}

	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(this, AECameraManager::StaticClass());
	if (IsValid(ManagerActor))
	{
		PivotActor = Cast<AECameraManager>(ManagerActor)->GetActiveCamera()->GetFollowTarget();
	}

	ControlledPawn = GetOwningPlayerController()->GetPawn();

	GetOwningPlayerController()->Possess(PhotoCamera);
	GetOwningPlayerController()->SetViewTargetWithBlend(PhotoCamera);

	PauseGame();
}

void AEPlayerCameraManager::PauseGame()
{
	bOriginalEnableDither = bEnableDither;
	bEnableDither = false;

	/** Set unpausable objects. */
	GetOwningPlayerController()->bShowMouseCursor = true;
	PhotoCamera->SetTickableWhenPaused(true);

	for (TSubclassOf<AActor> UnpaussableObjectClass : UnpausableObjects)
	{
		TArray<AActor*> RetrievedActors;
		UGameplayStatics::GetAllActorsOfClass(this, UnpaussableObjectClass, RetrievedActors);

		for (AActor* Actor : RetrievedActors)
		{
			if (IsValid(Actor))
			{
				Actor->SetTickableWhenPaused(true);
			}
		}
	}

	UGameplayStatics::SetGamePaused(this, true);
}

void AEPlayerCameraManager::UnpauseGame()
{
	bEnableDither = bOriginalEnableDither;

	/** Set unpausable objects. */
	GetOwningPlayerController()->bShowMouseCursor = false;
	PhotoCamera->SetTickableWhenPaused(false);

	for (TSubclassOf<AActor> UnpaussableObjectClass : UnpausableObjects)
	{
		TArray<AActor*> RetrievedActors;
		UGameplayStatics::GetAllActorsOfClass(this, UnpaussableObjectClass, RetrievedActors);

		for (AActor* Actor : RetrievedActors)
		{
			if (IsValid(Actor))
			{
				Actor->SetTickableWhenPaused(false);
			}
		}
	}

	UGameplayStatics::SetGamePaused(this, false);
}