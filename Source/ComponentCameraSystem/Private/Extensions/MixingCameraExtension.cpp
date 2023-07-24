// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/MixingCameraExtension.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/ECameraLibrary.h"

UMixingCameraExtension::UMixingCameraExtension()
{
	Stage = EStage::PreFollow;

	WeightUpdateScheme = EMixingCameraWeightUpdateScheme::Manual;
	MixScheme = EMixingCameraMixScheme::PositionOnly;
}

void UMixingCameraExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (GetOwningSettingComponent()->IsActive())
	{
		int ValidCameras = RefreshWeights();
		if (ValidCameras == 0)
		{
			return;
		}

		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float TotalWeight = 0.0f;

		for (int i = 0; i < Cameras.Num(); ++i)
		{
			if (IsValid(Cameras[i]))
			{
				switch (MixScheme)
				{
				case EMixingCameraMixScheme::PositionOnly:
				{
					Location += Weights[i] * Cameras[i]->GetActorLocation();
				}
				break;
				case EMixingCameraMixScheme::RotationOnly:
				{
					// Tricky to interpolate rotation. Vanilla slerp does not work.
					UpdateWeightedRotation(TotalWeight, Rotation, Weights[i], Cameras[i]);
				}
				break;
				case EMixingCameraMixScheme::Both:
				{
					Location += Weights[i] * Cameras[i]->GetActorLocation();
					UpdateWeightedRotation(TotalWeight, Rotation, Weights[i], Cameras[i]);

				}
				break;
				default:
				{ }
				}
			}
		}

		switch (MixScheme)
		{
		case EMixingCameraMixScheme::PositionOnly:
		{
			GetOwningActor()->SetActorLocation(Location);
		}
		break;
		case EMixingCameraMixScheme::RotationOnly:
		{
			GetOwningActor()->SetActorRotation(Rotation);
		}
		break;
		case EMixingCameraMixScheme::Both:
		{
			GetOwningActor()->SetActorLocation(Location);
			GetOwningActor()->SetActorRotation(Rotation);
		}
		break;
		default:
		{ }
		}
	}
}


void UMixingCameraExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	Cameras.Empty();

	for (FECameraClassWithTargets& CameraClassWithTargets : CameraClasses)
	{
		AECameraBase* Camera = CastChecked<AECameraBase>(GetWorld()->SpawnActor(CameraClassWithTargets.CameraClass));

		Camera->PrimaryActorTick.TickGroup = TG_PostPhysics;
		Camera->GetSettingsComponent()->SetForceActive(true);

		if (CameraClassWithTargets.FollowTarget.IsValid())
		{
			Camera->GetSettingsComponent()->SetFollowTarget(CameraClassWithTargets.FollowTarget.Get());
		}
		else
		{
			AActor* FollowTarget = UGameplayStatics::GetActorOfClass(this, CameraClassWithTargets.FollowTargetType);
			if (IsValid(FollowTarget))
			{
				Camera->GetSettingsComponent()->SetFollowTarget(FollowTarget);
			}
		}
		if (CameraClassWithTargets.AimTarget.IsValid())
		{
			Camera->GetSettingsComponent()->SetAimTarget(CameraClassWithTargets.AimTarget.Get());
		}
		else
		{
			AActor* AimTarget = UGameplayStatics::GetActorOfClass(this, CameraClassWithTargets.AimTargetType);
			if (IsValid(AimTarget))
			{
				Camera->GetSettingsComponent()->SetAimTarget(AimTarget);
			}
		}

		Cameras.Add(Camera);
	}

	for (AECameraBase* Camera : Cameras)
	{
		if (IsValid(Camera))
		{
			Camera->GetSettingsComponent()->BecomeViewTarget(PC, bPreserveState);
		}
	}
}

void UMixingCameraExtension::ResetOnEndViewTarget(APlayerController* PC)
{
	for (AECameraBase* Camera : Cameras)
	{
		if (IsValid(Camera))
		{
			Camera->GetSettingsComponent()->EndViewTarget(PC);
		}
	}
}

AECameraBase* UMixingCameraExtension::GetSubCameraOfClass(TSubclassOf<AECameraBase> CameraClass)
{
	for (AECameraBase* Camera : Cameras)
	{
		if (IsValid(Camera) && Camera->IsA(CameraClass))
		{
			return Camera;
		}
	}

	return nullptr;
}

AECameraBase* UMixingCameraExtension::GetSubCameraOfIndex(int index)
{
	return index >= Cameras.Num() ? nullptr : Cameras[index];
}

TArray<float> UMixingCameraExtension::NormalizeWeights(TArray<float> InWeights)
{
	float SquaredSum = 0.0f;
	TArray<float> NormalizedWeights;
	TArray<int> Validity;
	int ValidCameras = 0;

	for (int i = 0; i < Cameras.Num(); ++i)
	{
		Validity.Add(IsValid(Cameras[i]));
		if (IsValid(Cameras[i]))
		{
			ValidCameras += 1;
			SquaredSum += FMath::Square(InWeights[i]);
		}
	}

	if (ValidCameras == 0)
	{
		return InWeights;
	}

	for (int i = 0; i < Validity.Num(); ++i)
	{
		float NewWeight;

		if (SquaredSum != 0.0f)
		{
			NewWeight = InWeights[i] * InWeights[i] / SquaredSum;
		}
		else
		{
			NewWeight = 1.0f / ValidCameras;
		}

		NormalizedWeights.Add(NewWeight * Validity[i]);
	}

	return NormalizedWeights;
}

void UMixingCameraExtension::SetWeights(TArray<float> InWeights)
{
	Weights = InWeights;
}

int UMixingCameraExtension::RefreshWeights()
{
	int ValidCameras = 0;
	TArray<int> Validity;

	for (int i = 0; i < Cameras.Num(); ++i)
	{
		Validity.Add(IsValid(Cameras[i]));

		if (IsValid(Cameras[i]))
		{
			ValidCameras += 1;
		}
	}

	if (WeightUpdateScheme == EMixingCameraWeightUpdateScheme::Average)
	{
		if (ValidCameras != 0)
		{
			for (int i = 0; i < Validity.Num(); ++i)
			{
				Weights[i] = 1.0 / ValidCameras * Validity[i];
			}
		}
	}

	return ValidCameras;
}

void UMixingCameraExtension::UpdateWeightedRotation(float& TotalWeight, FRotator& Rotation, float CurrentWeight, AECameraBase* Camera)
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