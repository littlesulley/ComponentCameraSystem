// Copyright 2023 by Sulley. All Rights Reserved.


#include "Utils/ECameraLibrary.h"
#include "Utils/ESequencedCameraSetupActor.h"
#include "Components/ECameraComponentAim.h"
#include "Components/ControlAim.h"
#include "Cameras/EAnimatedCamera.h"
#include "Cameras/EKeyframedCamera.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraSubsystem.h"
#include "Core/EPlayerCameraManager.h"
#include "Extensions/AnimatedCameraExtension.h"
#include "Extensions/KeyframeExtension.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ScriptInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

std::vector<float> UECameraLibrary::Factorials = {1, 1, 2, 6, 24, 120, 720, 5040};

void UECameraLibrary::EasyDampVectorWithSameDampTime(const FDampParams DampParams,   // All methods
												     const FVector Input,            // All methods
												     const float DeltaSeconds,       // All methods
												     const float DampTime,           // All methods
												     FVector& Output,                // All methods
												     const FVector CurrentVelocity,  // Spring
												     FVector& OutVelocity,           // Spring
												     const FVector PreviousResidual, // ContinuousNaive
													 FVector& DeltaResidual          // Lowpass
											)
{
	EasyDamp(DampParams, Input[0], DeltaSeconds, DampTime, Output[0], DampParams.Frequency[0], DampParams.DampRatio[0], CurrentVelocity[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input[1], DeltaSeconds, DampTime, Output[1], DampParams.Frequency[1], DampParams.DampRatio[1], CurrentVelocity[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input[2], DeltaSeconds, DampTime, Output[2], DampParams.Frequency[2], DampParams.DampRatio[2], CurrentVelocity[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampRotatorWithSameDampTime(const FDampParams DampParams,   // All methods
												      const FRotator Input,           // All methods
												      const float DeltaSeconds,       // All methods
												      const float DampTime,           // All methods
												      FRotator& Output,               // All methods
												      const FVector CurrentVelocity,  // Spring
												      FVector& OutVelocity,           // Spring
												      const FVector PreviousResidual, // ContinuousNaive
													  FVector& DeltaResidual          // Lowpass
											)
{
	EasyDamp(DampParams, Input.Roll, DeltaSeconds, DampTime, Output.Roll, DampParams.Frequency[0], DampParams.DampRatio[0], CurrentVelocity[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input.Pitch, DeltaSeconds, DampTime, Output.Pitch, DampParams.Frequency[1], DampParams.DampRatio[1], CurrentVelocity[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input.Yaw, DeltaSeconds, DampTime, Output.Yaw, DampParams.Frequency[2], DampParams.DampRatio[2], CurrentVelocity[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampVectorWithDifferentDampTime(const FDampParams DampParams,   // All methods
														  const FVector Input,            // All methods
														  const float DeltaSeconds,       // All methods
														  const FVector DampTime,         // All methods
														  FVector& Output,                // All methods
														  const FVector CurrentVelocity,  // Spring
														  FVector& OutVelocity,           // Spring
														  const FVector PreviousResidual, // ContinuousNaive
													      FVector& DeltaResidual          // Lowpass
												)
{
	EasyDamp(DampParams, Input[0], DeltaSeconds, DampTime[0], Output[0], DampParams.Frequency[0], DampParams.DampRatio[0], CurrentVelocity[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input[1], DeltaSeconds, DampTime[1], Output[1], DampParams.Frequency[1], DampParams.DampRatio[1], CurrentVelocity[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input[2], DeltaSeconds, DampTime[2], Output[2], DampParams.Frequency[2], DampParams.DampRatio[2], CurrentVelocity[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampRotatorWithDifferentDampTime(const FDampParams DampParams,  // All methods
														  const FRotator Input,           // All methods
														  const float DeltaSeconds,       // All methods
														  const FVector DampTime,         // All methods
														  FRotator& Output,               // All methods
														  const FVector CurrentVelocity,  // Spring
														  FVector& OutVelocity,           // Spring
														  const FVector PreviousResidual, // ContinuousNaive
													      FVector& DeltaResidual          // Lowpass
												)
{
	EasyDamp(DampParams, Input.Roll, DeltaSeconds, DampTime[0], Output.Roll, DampParams.Frequency[0], DampParams.DampRatio[0], CurrentVelocity[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input.Pitch, DeltaSeconds, DampTime[1], Output.Pitch, DampParams.Frequency[1], DampParams.DampRatio[1], CurrentVelocity[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input.Yaw, DeltaSeconds, DampTime[2], Output.Yaw, DampParams.Frequency[2], DampParams.DampRatio[2], CurrentVelocity[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDamp(const FDampParams DampParams,   // All methods
							   const float Input,              // All methods
							   const float DeltaSeconds,       // All methods
							   const float DampTime,           // All methods
							   double& Output,                 // All methods
							   const float Frequency,		   // Spring
	                           const float DampRatio,          // Spring
							   const float CurrentVelocity,    // Spring
							   double& OutVelocity,            // Spring
							   const float PreviousResidual,   // ContinuousNaive
							   double& DeltaResidual           // LowPass
						)
{
	switch (DampParams.DampMethod)
	{
		case EDampMethod::Naive:
		{
			DamperValue(DampParams, DeltaSeconds, Input, DampTime, Output);
		}
		break;
		case EDampMethod::Simulate:
		{
			DamperValue(DampParams, DeltaSeconds, Input, DampTime, Output);
		}
		break;
		case EDampMethod::Spring:
		{
			SpringDampValue(Frequency, DampRatio, CurrentVelocity, OutVelocity, DeltaSeconds, Input, Output);
		}
		break;
		case EDampMethod::ContinuousNaive:
		{
			ContinuousDamperValue(DampParams, Input, PreviousResidual, DeltaSeconds, DampTime, Output);
		}
		break;
		case EDampMethod::RestrictedNaive:
		{
			RestrictionDampValue(DampParams, Input, PreviousResidual, 0.0f, DeltaSeconds, DampTime, Output);
		}
		break;
		case EDampMethod::SoftRestrictedNaive:
		{
			RestrictionDampValue(DampParams, Input, PreviousResidual, DampParams.Power, DeltaSeconds, DampTime, Output);
		}
		break;
		case EDampMethod::LowPassNaive:
		{
			LowpassDampValue(DampParams, Input, PreviousResidual, DeltaSeconds, DampTime, DeltaResidual, Output);
		}
		break;
		default:
			Output = Input;
		return;
	}
}

void UECameraLibrary::DamperValue(const FDampParams& DampParams, const float& DeltaSeconds, const float& Input, float DampTime, double& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	switch (DampParams.DampMethod)
	{
		case EDampMethod::Naive:
		{
			float lnResidual = FMath::Loge(DampParams.Residual);
			Output = Input * (1.0f - FMath::Exp(lnResidual * DeltaSeconds / DampTime));
		}
		break;
		case EDampMethod::Simulate:
		{
			const int n = 10; // Split DeltaSeconds into *n* equally distributed segments
			float lnResidual = FMath::Loge(DampParams.Residual);
			float ratio = FMath::Exp(lnResidual * DeltaSeconds / n / DampTime);
			Output = Input - Input / n * ratio * (1.0f - FMath::Exp(lnResidual * DeltaSeconds / DampTime)) / (1.0f - ratio);
		}
		break;
		default:
			Output = Input;
		return;
	}
}

void UECameraLibrary::NaiveDamperValue(const float& DeltaSeconds, const float& Input, double& Output, float DampTime, float Residual)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	float lnResidual = FMath::Loge(Residual);
	Output = Input * (1.0f - FMath::Exp(lnResidual * DeltaSeconds / DampTime));
}

void UECameraLibrary::DamperVectorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, float DampTime, FVector& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.X, DampTime, Output.X);
	DamperValue(DampParams, DeltaSeconds, Input.Y, DampTime, Output.Y);
	DamperValue(DampParams, DeltaSeconds, Input.Z, DampTime, Output.Z);
}

void UECameraLibrary::NaiveDamperVectorWithSameDampTime(const float& DeltaSeconds, const FVector& Input, FVector& Output, float DampTime, FVector Residual)
{
	NaiveDamperValue(DeltaSeconds, Input.X, Output.X, DampTime, Residual.X);
	NaiveDamperValue(DeltaSeconds, Input.Y, Output.Y, DampTime, Residual.Y);
	NaiveDamperValue(DeltaSeconds, Input.Z, Output.Z, DampTime, Residual.Z);
}

void UECameraLibrary::DamperVectorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, FVector DampTime, FVector& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.X, DampTime.X, Output.X);
	DamperValue(DampParams, DeltaSeconds, Input.Y, DampTime.Y, Output.Y);
	DamperValue(DampParams, DeltaSeconds, Input.Z, DampTime.Z, Output.Z);
}

void UECameraLibrary::DamperRotatorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, float DampTime, FRotator& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.Pitch, DampTime, Output.Pitch);
	DamperValue(DampParams, DeltaSeconds, Input.Yaw, DampTime, Output.Yaw);
	DamperValue(DampParams, DeltaSeconds, Input.Roll, DampTime, Output.Roll);
}

void UECameraLibrary::NaiveDamperRotatorWithSameDampTime(const float& DeltaSeconds, const FRotator& Input, FRotator& Output, float DampTime, FVector Residual)
{
	NaiveDamperValue(DeltaSeconds, Input.Pitch, Output.Pitch, DampTime, Residual.X);
	NaiveDamperValue(DeltaSeconds, Input.Yaw, Output.Yaw, DampTime, Residual.Y);
	NaiveDamperValue(DeltaSeconds, Input.Roll, Output.Roll, DampTime, Residual.Z);
}

void UECameraLibrary::DamperRotatorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, FVector DampTime, FRotator& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.Roll, DampTime.X, Output.Roll);
	DamperValue(DampParams, DeltaSeconds, Input.Pitch, DampTime.Y, Output.Pitch);
	DamperValue(DampParams, DeltaSeconds, Input.Yaw, DampTime.Z, Output.Yaw);
}

void UECameraLibrary::SpringDampValue(const float& Frequency, const float& DampRatio, const float& CurrentVelocity, double& OutVelocity, const float& DeltaSeconds, const float& Input, double& Output)
{
	// Underdamped
	if (DampRatio < 1)
	{
		float SqrtOfOneMinusSquare = FMath::Sqrt(1.0 - DampRatio * DampRatio);
		float OmegaDotSqrt = Frequency * SqrtOfOneMinusSquare;
		float Inner = OmegaDotSqrt * DeltaSeconds;
		float OmegaDotZeta = Frequency * DampRatio;
		float Cosine = FMath::Cos(Inner);
		float Sine = FMath::Sin(Inner);

		float C1 = Input;
		float C2 = (CurrentVelocity + OmegaDotZeta * Input) / (OmegaDotSqrt + 1e-5f);
		float Decay = FMath::Exp(-OmegaDotZeta * DeltaSeconds);
		float X = C1 * Decay * Cosine + C2 * Decay * Sine;
		OutVelocity = -OmegaDotZeta * X + (CurrentVelocity + OmegaDotZeta * Input) * Decay * Cosine - Input * OmegaDotSqrt * Decay * Sine;
		Output = Input - X;
	}
	// Critically damped
	else if (DampRatio == 1)
	{
		float X = Input * FMath::Exp(-Frequency * DeltaSeconds);
		OutVelocity = -Frequency * X;
		Output = Input - X;
	}
	// Overdamped
	else if (DampRatio > 1)
	{
		float SqrtOfSquareMinusOne = FMath::Sqrt(DampRatio * DampRatio - 1.0);
		float ZetaPlusSqrt = DampRatio + SqrtOfSquareMinusOne;
		float ZetaMinusSqrt = DampRatio - SqrtOfSquareMinusOne;
		float NegOmegaDotPlus = -Frequency * ZetaPlusSqrt;
		float NegOmegaDotMinus = -Frequency * ZetaMinusSqrt;

		float C1 = (-CurrentVelocity / Frequency - ZetaMinusSqrt * Input) / (2.0 * SqrtOfSquareMinusOne + 1e-5f);
		float C2 = Input - C1;
		float T1 = C1 * FMath::Exp(DeltaSeconds * NegOmegaDotPlus);
		float T2 = C2 * FMath::Exp(DeltaSeconds * NegOmegaDotMinus);
		float X = T1 + T2;
		OutVelocity = NegOmegaDotPlus * T1 + NegOmegaDotMinus * T2;
		Output = Input - X;
	}
	// Undamped
	else
	{
		float Cosine = FMath::Cos(Frequency * DeltaSeconds);
		float Sine = FMath::Sin(Frequency * DeltaSeconds);
		float X = CurrentVelocity / Frequency * Sine + Input * Cosine;
		OutVelocity = CurrentVelocity * Cosine - Frequency * Input * Sine;
		Output = Input - X;
	}
}

void UECameraLibrary::ContinuousDamperVectorWithSameDampTime(const FDampParams& DampParams, const FVector& Input, const FVector& PreviousResidual, const float& DeltaSeconds, float DampTime, FVector& Output)
{
	ContinuousDamperValue(DampParams, Input[0], PreviousResidual[0], DeltaSeconds, DampTime, Output[0]);
	ContinuousDamperValue(DampParams, Input[1], PreviousResidual[1], DeltaSeconds, DampTime, Output[1]);
	ContinuousDamperValue(DampParams, Input[2], PreviousResidual[2], DeltaSeconds, DampTime, Output[2]);
}

void UECameraLibrary::ContinuousDamperVectorWithDifferentDampTime(const FDampParams& DampParams, const FVector& Input, const FVector& PreviousResidual, const float& DeltaSeconds, FVector DampTime, FVector& Output)
{
	ContinuousDamperValue(DampParams, Input[0], PreviousResidual[0], DeltaSeconds, DampTime[0], Output[0]);
	ContinuousDamperValue(DampParams, Input[1], PreviousResidual[1], DeltaSeconds, DampTime[1], Output[1]);
	ContinuousDamperValue(DampParams, Input[2], PreviousResidual[2], DeltaSeconds, DampTime[2], Output[2]);
}

void UECameraLibrary::ContinuousDamperValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, double& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	if (FMath::Abs(Input) < 0.01)
	{
		DamperValue(DampParams, DeltaSeconds, Input, DampTime, Output);
		return;
	}

	int Order = DampParams.Order;
	float Residual = DampParams.Residual;

	float Derivative = FMath::Loge(Residual) / DampTime;
	float Speed = (Input - PreviousResidual) / DeltaSeconds;
	float VelocityTerm = PreviousResidual * Derivative + Speed;

	float CoefficientTerm = 0.0f;
	for (int i = 1; i <= Order; ++i)
	{
		CoefficientTerm += FMath::Pow((double)Derivative, i - 1) * FMath::Pow((double)DeltaSeconds, i) / Factorials.at(i);
	}
	float DeltaResidual = VelocityTerm * CoefficientTerm;
	
	Output = Input - (PreviousResidual + DeltaResidual);
}

void UECameraLibrary::RestrictionDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const int& Power, const float& DeltaSeconds, float DampTime, double& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	if (FMath::Abs(Input) < UE_SMALL_NUMBER || PreviousResidual / Input <= 0)
	{
		DamperValue(DampParams, DeltaSeconds, Input, DampTime, Output);
		return;
	}

	float Tolerance = DampParams.Tolerance;
	float lnResidual = FMath::Loge(DampParams.Residual);
	float Alpha = FMath::Loge(PreviousResidual / Input) * DampTime / lnResidual;
	float Ratio = DeltaSeconds / Alpha;
	float NewDeltaSeconds = DeltaSeconds;

	if (Ratio >= 1.0f - Tolerance && Ratio <= 1.0f + Tolerance)
	{
		if (Power == 0.0f)
		{
			NewDeltaSeconds = Alpha;
		}
		else
		{
			float Delta = Ratio <= 1.0f ? 1.0f - Ratio : Ratio - 1.0f;
			Delta = FMath::Exp(Delta * FMath::Loge(1.0f + Tolerance) / Tolerance ) - 1;
			Delta = Delta * FMath::Pow(Delta / Tolerance, Power);

			NewDeltaSeconds = Ratio <= 1.0f ? Alpha * (1.0f - Delta) : Alpha * (1.0f + Delta);
		}
	}

	Output = Input * (1.0f - FMath::Exp(lnResidual * NewDeltaSeconds / DampTime));
}

void UECameraLibrary::LowpassDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, double& DeltaResidual, double& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	if (FMath::Abs(Input) < UE_SMALL_NUMBER || PreviousResidual / Input <= 0)
	{
		DamperValue(DampParams, DeltaSeconds, Input, DampTime, Output);
		return;
	}

	float Tolerance = DampParams.Tolerance;
	float lnResidual = FMath::Loge(DampParams.Residual);
	float CurrentResidual = Input * FMath::Exp(lnResidual * DeltaSeconds / DampTime);
	float CurrentDeltaResidual = CurrentResidual - PreviousResidual;

	float Alpha = FMath::Loge(PreviousResidual / Input) * DampTime / lnResidual;
	float Ratio = DeltaSeconds / Alpha;

	if (Ratio >= 1.0f - Tolerance && Ratio <= 1.0f + Tolerance)
	{
		float Beta = DampParams.LowpassBeta;
		DeltaResidual = (1 - Beta) * DeltaResidual + Beta * CurrentDeltaResidual;

		Output = Input - (DeltaResidual + PreviousResidual);
	}
	else
	{
		Output = Input - CurrentResidual;
	}
}

float UECameraLibrary::FastAtan(float x)
{
	float Z = FMath::Abs(x);
	float W = Z > 1.0f ? 1.0f / Z : Z;
	float Pi = 3.14159265359;
	float Y = (Pi / 4.0f) * W - W * (W - 1) * (0.2447f + 0.0663f * W);
	return Z > 1.0 ? Pi / 2.0 - Y : Y;
}

void UECameraLibrary::DamperQuaternion(const FQuat& Quat1, const FQuat& Quat2, const float& DeltaSeconds, float DampTime, FQuat& Output)
{
	double T;
	NaiveDamperValue(DeltaSeconds, 1, T, DampTime);
	Output = FQuat::Slerp(Quat1, Quat2, T);
}

FVector UECameraLibrary::GetLocalSpacePosition(const AActor* Camera, const FVector& InputPosition)
{
	FVector Diff = InputPosition - Camera->GetActorLocation();

	FVector ForwardVector = Camera->GetActorForwardVector();
	FVector RightVector = Camera->GetActorRightVector();
	FVector UpVector = Camera->GetActorUpVector();

	FVector LocalSpaceFollowPosition =
		UKismetMathLibrary::MakeVector(ForwardVector.X, RightVector.X, UpVector.X) * Diff.X +
		UKismetMathLibrary::MakeVector(ForwardVector.Y, RightVector.Y, UpVector.Y) * Diff.Y +
		UKismetMathLibrary::MakeVector(ForwardVector.Z, RightVector.Z, UpVector.Z) * Diff.Z;

	return LocalSpaceFollowPosition;
}

FVector UECameraLibrary::GetLocalSpacePositionWithVectors(const FVector& PivotPosition, const FVector& ForwardVector, const FVector& RightVector, const FVector& UpVector, const FVector& InputPosition)
{
	FVector Diff = InputPosition - PivotPosition;

	FVector LocalSpaceFollowPosition =
		UKismetMathLibrary::MakeVector(ForwardVector.X, RightVector.X, UpVector.X) * Diff.X +
		UKismetMathLibrary::MakeVector(ForwardVector.Y, RightVector.Y, UpVector.Y) * Diff.Y +
		UKismetMathLibrary::MakeVector(ForwardVector.Z, RightVector.Z, UpVector.Z) * Diff.Z;

	return LocalSpaceFollowPosition;
}

AECameraBase* UECameraLibrary::InternalCallCamera(
								const UObject* WorldContextObject,        // World context object.
								TSubclassOf<AECameraBase> CameraClass,    // Camera class inherited from AECameraClass.
								FVector SpawnLocation,                    // Spawn location, default is origin.
								FRotator SpawnRotation,                   // Spawn rotation, default is identity.
								AActor* FollowTarget,                     // Follow target, can be null.
								AActor* AimTarget,                        // Aim target, can be null.
								FName FollowSocket,                       // Socket name for follow target, can be none.
								FName AimSocket,                          // Socket name for aim target, can be none.
								USceneComponent* FollowSceneComponent,    // Scene component for follow target, can be none.
								USceneComponent* AimSceneComponent,       // Scene component for aim target, can be none.
								float BlendTime,                          // Blend time from prior camera.
								enum EViewTargetBlendFunction BlendFunc,  // Blend function.
								float BlendExp,							  // Blend exponential.
								bool bLockOutgoing,					  	  // Whether to lock outgoing frame.
								bool bIsTransitory,						  // Whether it is transitory.
								float LifeTime,							  // The life time.
								bool bPreserveState,				      // Whether to preserve camera state.
								bool bCheckClass,                         // Whether to check input camera class.
								AActor* ParentCamera                      // Parent camera
					)
{
	if (!CameraClass)
	{
		return nullptr;
	}

	if (bCheckClass)
	{
		AECameraBase* ActiveCamera = GetActiveCamera(WorldContextObject);
		if (IsValid(ActiveCamera) && ActiveCamera->GetClass() == CameraClass->GetOwnerClass() && !ActiveCamera->bIsTransitory && !bIsTransitory)
		{
			return ActiveCamera;
		}
	}

	AECameraBase* Camera;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World != nullptr)
	{
		Camera = CastChecked<AECameraBase>(World->SpawnActor(CameraClass));
	}
	else
	{
		return nullptr;
	}

	Camera->GetSettingsComponent()->SetFollowTarget(FollowTarget);
	Camera->GetSettingsComponent()->SetFollowSocket(FollowSocket);
	Camera->GetSettingsComponent()->SetFollowSceneComponent(FollowSceneComponent);
	Camera->GetSettingsComponent()->SetAimTarget(AimTarget);
	Camera->GetSettingsComponent()->SetAimSocket(AimSocket);
	Camera->GetSettingsComponent()->SetAimSceneComponent(AimSceneComponent);

	/** Set camera location and rotation. */
	Camera->SetActorLocation(SpawnLocation);
	Camera->SetActorRotation(SpawnRotation);

	/** Prepared to blend to the new camera. */
	if (World != nullptr)
	{
		/** Initialization. */
		Camera->ResetOnBecomeViewTarget(bIsTransitory, LifeTime, bPreserveState, ParentCamera);

		/** Blend to new camera. This will automatically call old camera's EndViewTarget and new camera's BecomeViewTarget. */
		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		PC->SetViewTargetWithBlend(Camera, BlendTime, BlendFunc, BlendExp, bLockOutgoing);
	}

	return Camera;
}

AECameraBase* UECameraLibrary::CallCamera(const UObject* WorldContextObject,        // World context object.
										  TSubclassOf<AECameraBase> CameraClass,    // Camera class inherited from AECameraClass.
								          FVector SpawnLocation,                    // Spawn location, default is origin.
										  FRotator SpawnRotation,                   // Spawn rotation, default is identity.
										  AActor* FollowTarget,                     // Follow target, can be null.
									      AActor* AimTarget,                        // Aim target, can be null.
										  FName FollowSocket,                       // Socket name for follow target, can be none.
										  FName AimSocket,                          // Socket name for aim target, can be none.
										  float BlendTime,                          // Blend time from prior camera.
										  enum EViewTargetBlendFunction BlendFunc,  // Blend function.
										  float BlendExp,							// Blend exponential.
										  bool bLockOutgoing,						// Whether to lock outgoing frame.
	                                      bool bIsTransitory,						// Whether it is transitory.
										  float LifeTime,							// The life time.
										  bool bPreserveState,						// Whether to preserve camera state.
										  bool bCheckClass,                         // Whether to check input camera class.
										  AActor* ParentCamera                      // Parent camera
										)
{
	return InternalCallCamera(
		WorldContextObject,
		CameraClass,
		SpawnLocation,
		SpawnRotation,
		FollowTarget,
		AimTarget,
		FollowSocket,
		AimSocket,
		nullptr,
		nullptr,
		BlendTime,
		BlendFunc,
		BlendExp,
		bLockOutgoing,
		bIsTransitory,
		LifeTime,
		bPreserveState,
		bCheckClass,
		ParentCamera
	);
}

AECameraBase* UECameraLibrary::CallCameraWithSceneComponent(
										  const UObject* WorldContextObject,        // World context object.
										  TSubclassOf<AECameraBase> CameraClass,    // Camera class inherited from AECameraClass.
								          FVector SpawnLocation,                    // Spawn location, default is origin.
										  FRotator SpawnRotation,                   // Spawn rotation, default is identity.
										  AActor* FollowTarget,                     // Follow target, can be null.
									      AActor* AimTarget,                        // Aim target, can be null.
										  USceneComponent* FollowSceneComponent,    // Scene component for follow target, can be none.
										  USceneComponent* AimSceneComponent,       // Scene component for aim target, can be none.
										  float BlendTime,                          // Blend time from prior camera.
										  enum EViewTargetBlendFunction BlendFunc,  // Blend function.
										  float BlendExp,							// Blend exponential.
										  bool bLockOutgoing,						// Whether to lock outgoing frame.
	                                      bool bIsTransitory,						// Whether it is transitory.
										  float LifeTime,							// The life time.
										  bool bPreserveState,						// Whether to preserve camera state.
										  bool bCheckClass,                         // Whether to check input camera class.
										  AActor* ParentCamera                      // Parent camera
										)
{
	return InternalCallCamera(
		WorldContextObject,
		CameraClass,
		SpawnLocation,
		SpawnRotation,
		FollowTarget,
		AimTarget,
		FName("None"),
		FName("None"),
		FollowSceneComponent,
		AimSceneComponent,
		BlendTime,
		BlendFunc,
		BlendExp,
		bLockOutgoing,
		bIsTransitory,
		LifeTime,
		bPreserveState,
		bCheckClass,
		ParentCamera
	);
}

AECameraBase* UECameraLibrary::CallAnimatedCamera(const UObject* WorldContextObject,            // World context object.
												  UAnimSequence* AnimToPlay,                    // Animation sequence to play.
												  AActor* CoordinateActor,					    // In which actor's local space you want to play the camera animation.
												  const FTransform& Coordinate,                 // In which reference frame you want to play the camera animation.
												  FVector PositionOffset,						// Position offset, in reference space.
												  float BlendTime,								// Blend time from prior camera.
												  enum EViewTargetBlendFunction BlendFunc,		// Blend function
												  float BlendExp,								// Blend exponential.
												  bool bLockOutgoing)							// Whether to lock outgoing frame.
{
	if (!IsValid(AnimToPlay))
	{
		return nullptr;
	}

	AEAnimatedCamera* Camera;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World != nullptr)
	{
		Camera = CastChecked<AEAnimatedCamera>(World->SpawnActor(AEAnimatedCamera::StaticClass()));
	}
	else
	{
		return nullptr;
	}

	UAnimatedCameraExtension* AnimatedCameraExtension = Cast<UAnimatedCameraExtension>(Camera->GetSettingsComponent()->GetExtensions()[0]);
	if (AnimatedCameraExtension)
	{
		AnimatedCameraExtension->SetAnim(AnimToPlay);
		AnimatedCameraExtension->SetRefActor(CoordinateActor);
		AnimatedCameraExtension->SetRef(Coordinate);
		AnimatedCameraExtension->SetOffset(PositionOffset);
	}

	if (World != nullptr)
	{
		Camera->ResetOnBecomeViewTarget(true, 99999.f, false);

		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		PC->SetViewTargetWithBlend(Camera, BlendTime, BlendFunc, BlendExp, bLockOutgoing);
	}

	return Camera;
}

AECameraBase* UECameraLibrary::CallKeyframedCamera(const UObject* WorldContextObject,                    // World context object.
												   TSubclassOf<AEKeyframedCamera> KeyframedCamera,       // Keyframed camera to play.
												   AActor* CoordinateActor,                              // In which actor's local space you want to apply keyframes.
												   FName CoordinateSocket,                               // CoordinateActor's socket the camera is based on.
												   const FTransform& Coordinate,						 // In which reference frame you want to apply keyframes.
												   bool bCoordinateLocationOnly,						 // Whether to apply only positions in coordiante space.
												   FVector LocationOffset,							     // Position offset, in coordiante space.
												   AActor* AimOverride,									 // Actor to aim at.
												   FName AimSocket,										 // AimOverride's socket to aim at.
												   float BlendTime,										 // Blend time from prior camera.
												   enum EViewTargetBlendFunction BlendFunc,				 // Blend function
												   float BlendExp, 										 // Blend exponential.
												   bool bLockOutgoing)									 // Whether to lock outgoing frame.
{
	if (!IsValid(KeyframedCamera))
	{
		return nullptr;
	}

	AEKeyframedCamera* Camera;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World != nullptr)
	{
		Camera = CastChecked<AEKeyframedCamera>(World->SpawnActor(KeyframedCamera));
	}
	else
	{
		return nullptr;
	}

	UKeyframeExtension* Extension = Cast<UKeyframeExtension>(Camera->GetExtensionOfClass(UKeyframeExtension::StaticClass()));

	Extension->CoordinateActor = CoordinateActor;
	Extension->CoordinateSocket = CoordinateSocket;
	Extension->Coordinate = Coordinate;
	Extension->bCoordinateLocationOnly = bCoordinateLocationOnly;
	Extension->LocationOffset = LocationOffset;
	Extension->AimOverride = AimOverride;
	Extension->AimSocket = AimSocket;

	if (World != nullptr)
	{
		Camera->ResetOnBecomeViewTarget(true, 99999.f, false);

		APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
		PC->SetViewTargetWithBlend(Camera, BlendTime, BlendFunc, BlendExp, bLockOutgoing);
	}

	return Camera;
}

AESequencedCameraSetupActor* UECameraLibrary::CallSequencedCamera(const UObject* WorldContextObject, TSubclassOf<AESequencedCameraSetupActor> SequencedCameraClass)
{
	AESequencedCameraSetupActor* SequencedCameraSetupActor = nullptr;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World != nullptr)
	{
		SequencedCameraSetupActor = CastChecked<AESequencedCameraSetupActor>(World->SpawnActor(SequencedCameraClass));
	}

	return SequencedCameraSetupActor;
}

void UECameraLibrary::TerminateActiveCamera(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (UECameraSubsystem* Subsystem = GameInstance->GetSubsystem<UECameraSubsystem>())
		{
			if (IsValid(Subsystem))
			{
				Subsystem->TerminateActiveCamera();
			}
		}
	}
}

AECameraBase* UECameraLibrary::GetActiveCamera(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (UECameraSubsystem* Subsystem = GameInstance->GetSubsystem<UECameraSubsystem>())
		{
			if (IsValid(Subsystem))
			{
				return Subsystem->GetActiveCamera();
			}
		}
	}

	return nullptr;
}

AEPlayerCameraManager* UECameraLibrary::GetEPlayerCameraManager(const UObject* WorldContextObject, int32 index)
{
	APlayerCameraManager* Manager = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, index);
	return Cast<AEPlayerCameraManager>(Manager);
}

FVector UECameraLibrary::GetPositionWithLocalOffset(AActor* TargetActor, const FVector& Offset)
{
	FVector ActorLocation = TargetActor->GetActorLocation();
	FRotator ActorRotation = TargetActor->GetActorRotation();
	FVector LocalOffset = UKismetMathLibrary::GreaterGreater_VectorRotator(Offset, ActorRotation);

	return ActorLocation + LocalOffset;
}

FVector UECameraLibrary::GetPositionWithLocalRotatedOffset(const FVector& Position, const FRotator& Rotation, const FVector& Offset)
{
	return Position + UKismetMathLibrary::GreaterGreater_VectorRotator(Offset, Rotation);
}

void UECameraLibrary::AddBlendable(const UObject* WorldContextObject, const TScriptInterface<IBlendableInterface>& InBlendableObject, const float InWeight, const float InBlendInTime, const float InDuration, const float InBlendOutTime)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);
	if (IsValid(Manager))
	{
		Manager->AddBlendable(InBlendableObject, InWeight, InBlendInTime, InDuration, InBlendOutTime);
	}
}

void UECameraLibrary::RemoveBlendable(const UObject* WorldContextObject, const TScriptInterface<IBlendableInterface>& InBlendableObject)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);
	if (IsValid(Manager))
	{
		Manager->RemoveBlendable(InBlendableObject);
	}
}

UCameraShakeBase* UECameraLibrary::EasyStartCameraShake(const UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, FPackedOscillationParams ShakeParams, float Scale, ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot, bool bSingleInstance)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);

	if (IsValid(Manager))
	{
		return Manager->EasyStartCameraShake(ShakeClass, ShakeParams, Scale, PlaySpace, UserPlaySpaceRot, bSingleInstance);
	}
	else
	{
		return nullptr;
	}
}

UCameraShakeBase* UECameraLibrary::EasyStartCameraShakeFromSource(const UObject* WorldContextObject, TSubclassOf<UCameraShakeBase> ShakeClass, FPackedOscillationParams ShakeParams, AActor* SpawnActor, FVector SpawnLocation, ECameraShakeAttenuation Attenuation, float InnerAttenuationRadius, float OuterAttenuationRadius, float Scale, ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot, bool bSingleInstance)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);

	if (IsValid(Manager))
	{
		return Manager->EasyStartCameraShakeFromSource(ShakeClass, ShakeParams, SpawnActor, SpawnLocation, Attenuation, InnerAttenuationRadius, OuterAttenuationRadius, Scale, PlaySpace, UserPlaySpaceRot, bSingleInstance);
	}
	else
	{
		return nullptr;
	}
}

void UECameraLibrary::EasyStartCameraFade(const UObject* WorldContextObject, float FromAlpha, float ToAlpha, float FadeInTime, TEnumAsByte<EEasingFunc::Type> FadeInFunc, float Duration, float FadeOutTime, TEnumAsByte<EEasingFunc::Type> FadeOutFunc, FLinearColor Color)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);
	if (IsValid(Manager))
	{
		Manager->EasyStartCameraFade(FromAlpha, ToAlpha, FadeInTime, FadeInFunc, Duration, FadeOutTime, FadeOutFunc, Color);
	}
}

void UECameraLibrary::EasyStopCameraFade(const UObject* WorldContextObject, float StopAlpha)
{
	AEPlayerCameraManager* Manager = GetEPlayerCameraManager(WorldContextObject, 0);
	if (IsValid(Manager))
	{
		Manager->EasyStopCameraFade(StopAlpha);
	}
}

void UECameraLibrary::EasyStartRecentering(const UObject* WorldContextObject, float Duration, TEnumAsByte<EEasingFunc::Type> Func, float Exp)
{
	AECameraBase* CurrentCamera = GetActiveCamera(WorldContextObject);

	if (IsValid(CurrentCamera) 
		&& IsValid(CurrentCamera->GetSettingsComponent()->GetAimComponent()) 
		&& CurrentCamera->GetSettingsComponent()->GetAimComponent()->IsA<UControlAim>())
	{
		Cast<UControlAim>(CurrentCamera->GetSettingsComponent()->GetAimComponent())->StartRecentering(Duration, Func, Exp);
	}
}

void UECameraLibrary::EasyStopRecentering(const UObject* WorldContextObject)
{
	AECameraBase* CurrentCamera = GetActiveCamera(WorldContextObject);

	if (IsValid(CurrentCamera)
		&& IsValid(CurrentCamera->GetSettingsComponent()->GetAimComponent())
		&& CurrentCamera->GetSettingsComponent()->GetAimComponent()->IsA<UControlAim>())
	{
		Cast<UControlAim>(CurrentCamera->GetSettingsComponent()->GetAimComponent())->StopRecentering();
	}
}