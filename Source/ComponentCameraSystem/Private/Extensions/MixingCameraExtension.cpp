// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/MixingCameraExtension.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/ECameraLibrary.h"

UMixingCameraExtension::UMixingCameraExtension()
{
	Stage = EStage::PreFollow;

	WeightUpdateScheme = EMixingCameraWeightUpdateScheme::Manual;
	MixScheme = EMixingCameraMixScheme::PositionOnly;
	MixRotationMethod = EMixingCameraMixRotationMethod::Eigenvalue;
	CircularEpsilon = 0.25f;
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

		switch (MixScheme)
		{
		case EMixingCameraMixScheme::PositionOnly:
		{
			Location = GetWeightedPosition();
			GetOwningActor()->SetActorLocation(Location);
		}
		break;
		case EMixingCameraMixScheme::RotationOnly:
		{
			Rotation = GetWeightedRotation();
			GetOwningActor()->SetActorRotation(Rotation);
		}
		break;
		case EMixingCameraMixScheme::Both:
		{
			Location = GetWeightedPosition();
			Rotation = GetWeightedRotation();
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

FVector UMixingCameraExtension::GetWeightedPosition()
{
	FVector Location = FVector::ZeroVector;

	for (int i = 0; i < Cameras.Num(); ++i)
	{
		if (IsValid(Cameras[i]))
		{
			Location += Weights[i] * Cameras[i]->GetActorLocation();
		}
	}

	return Location;
}

FRotator UMixingCameraExtension::GetWeightedRotation()
{
	FRotator Rotation = FRotator::ZeroRotator;

	if (MixRotationMethod == EMixingCameraMixRotationMethod::Eigenvalue)
	{
		Rotation = AverageRotations().Rotator();
	}
	else if (MixRotationMethod == EMixingCameraMixRotationMethod::Circular)
	{
		Rotation = CircularAverageRotations();
	}
	else
	{
		float TotalWeight = 0.0f;

		for (int i = 0; i < Cameras.Num(); ++i)
		{
			if (IsValid(Cameras[i]))
			{
				UpdateWeightedRotation(TotalWeight, Rotation, Weights[i], Cameras[i]);
			}
		}
	}

	return Rotation;
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

// Ref https://www.acsu.buffalo.edu/%7Ejohnc/ave_quat07.pdf
FQuat UMixingCameraExtension::AverageRotations()
{
	// Must initialize as all-zeros
	FMatrix Accumulated = FMatrix(
		FPlane(0, 0, 0, 0),
		FPlane(0, 0, 0, 0),
		FPlane(0, 0, 0, 0),
		FPlane(0, 0, 0, 0)
	);

	for (int i = 0; i < Cameras.Num(); ++i)
	{
		FQuat Q = Cameras[i]->GetActorQuat();

		FMatrix M = FMatrix(
			FPlane(Q.X * Q.X, Q.X * Q.Y, Q.X * Q.Z, Q.X * Q.W),
			FPlane(Q.Y * Q.X, Q.Y * Q.Y, Q.Y * Q.Z, Q.Y * Q.W),
			FPlane(Q.Z * Q.X, Q.Z * Q.Y, Q.Z * Q.Z, Q.Z * Q.W),
			FPlane(Q.W * Q.X, Q.W * Q.Y, Q.W * Q.Z, Q.W * Q.W)
		);

		Accumulated += M * Weights[i];
	}
	
	FVector4 V = FindEigenvectorUsingPI(Accumulated, FVector4(0, 0, 0, 1), 64);
	FQuat Q = FQuat(V.X, V.Y, V.Z, V.W);
	
	return Q.W < 0 ? -Q : Q;
}

// Ref https://sulley.cc/2024/01/11/20/06/#sc-lerp-smoothed-c-lerp
FRotator UMixingCameraExtension::CircularAverageRotations()
{
	float SumSinYaw = 0.f, SumCosYaw = 0.f;
	float SumSinPitch = 0.f, SumCosPitch = 0.f;
	float SumSinRoll = 0.f, SumCosRoll = 0.f;

	for (int i = 0; i < Cameras.Num(); ++i)
	{
		FRotator Q = Cameras[i]->GetActorRotation();
		
		SumSinYaw   += Weights[i] * UKismetMathLibrary::DegSin(Q.Yaw);
		SumCosYaw   += Weights[i] * UKismetMathLibrary::DegCos(Q.Yaw);
		SumSinPitch += Weights[i] * UKismetMathLibrary::DegSin(Q.Pitch);
		SumCosPitch += Weights[i] * UKismetMathLibrary::DegCos(Q.Pitch);
		SumSinRoll  += Weights[i] * UKismetMathLibrary::DegSin(Q.Roll);
		SumCosRoll  += Weights[i] * UKismetMathLibrary::DegCos(Q.Roll);
	}

	return FRotator(
		FMath::RadiansToDegrees(UKismetMathLibrary::Atan2(SumSinPitch, SumCosPitch + CircularEpsilon)),
		FMath::RadiansToDegrees(UKismetMathLibrary::Atan2(SumSinYaw, SumCosYaw + CircularEpsilon)),
		FMath::RadiansToDegrees(UKismetMathLibrary::Atan2(SumSinRoll, SumCosRoll + CircularEpsilon))
	);
}

// Power iteration to find eigenvector. Ref https://en.wikipedia.org/wiki/Power_iteration 
// Rayleigh quotient iteration converges faster, but involving computing matrix inverse. Ref https://en.wikipedia.org/wiki/Rayleigh_quotient_iteration
FVector4 UMixingCameraExtension::FindEigenvectorUsingPI(const FMatrix& M, const FVector4& V, const int Steps, const float Epsilon)
{
	FVector4 EigenVector = V;
	float EigenValue = M.TransformFVector4(EigenVector).X / EigenVector.X;

	for (int i = 0; i < Steps; ++i)
	{
		FVector4 Mul = M.TransformFVector4(EigenVector);

		FVector4 NewEigenVector = NormalizeVector4(Mul);
		float NewEigenValue = M.TransformFVector4(NewEigenVector).X / NewEigenVector.X;

		if (FMath::Abs(EigenValue - NewEigenValue) < Epsilon)
		{
			break;
		}

		EigenVector = NewEigenVector;
		EigenValue = NewEigenValue;
	}

	return EigenVector;
}

FVector4 UMixingCameraExtension::NormalizeVector4(const FVector4& V, float Tolerance)
{
	float SquareSum = V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W;
	if (SquareSum > Tolerance)
	{
		const float Scale = FMath::InvSqrt(SquareSum);
		return FVector4(V.X * Scale, V.Y * Scale, V.Z * Scale, V.W * Scale);
	}
	return V;
}