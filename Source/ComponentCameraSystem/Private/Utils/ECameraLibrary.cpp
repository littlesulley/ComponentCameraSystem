// Copyright 2023 by Sulley. All Rights Reserved.


#include "Utils/ECameraLibrary.h"
#include "Utils/ESequencedCameraSetupActor.h"
#include "Cameras/EAnimatedCamera.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraManager.h"
#include "Core/EPlayerCameraManager.h"
#include "Extensions/AnimatedCameraExtension.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ScriptInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

int UECameraLibrary::Factorials[] = { 1, 1, 2, 6, 24, 120, 720, 5040 };

void UECameraLibrary::EasyDampVectorWithSameDampTime(const FDampParams DampParams,   // All methods
												     const FVector Input,            // All methods
												     const float DeltaSeconds,       // All methods
												     const float DampTime,           // All methods
												     FVector& Output,                // All methods
												     const FVector TemporalInput,    // Spring
												     const FVector SpatialInput,     // Spring
												     const FVector CurrentVelocity,  // ExactSpring
												     const FVector TargetVector,     // ExactSpring
												     const FVector TargetVelocity,   // ExactSpring
												     FVector& OutVelocity,           // ExactSpring
												     const FVector PreviousResidual, // ContinuousNaive
													 FVector& DeltaResidual          // Lowpass
											)
{
	EasyDamp(DampParams, Input[0], DeltaSeconds, DampTime, Output[0], TemporalInput[0], SpatialInput[0], DampParams.SpringCoefficient[0], CurrentVelocity[0], TargetVector[0], TargetVelocity[0], DampParams.DampRatio[0], DampParams.HalfLife[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input[1], DeltaSeconds, DampTime, Output[1], TemporalInput[1], SpatialInput[1], DampParams.SpringCoefficient[1], CurrentVelocity[1], TargetVector[1], TargetVelocity[1], DampParams.DampRatio[1], DampParams.HalfLife[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input[2], DeltaSeconds, DampTime, Output[2], TemporalInput[2], SpatialInput[2], DampParams.SpringCoefficient[2], CurrentVelocity[2], TargetVector[2], TargetVelocity[2], DampParams.DampRatio[2], DampParams.HalfLife[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampRotatorWithSameDampTime(const FDampParams DampParams,   // All methods
												      const FRotator Input,           // All methods
												      const float DeltaSeconds,       // All methods
												      const float DampTime,           // All methods
												      FRotator& Output,               // All methods
												      const FVector TemporalInput,    // Spring
												      const FVector SpatialInput,     // Spring
												      const FVector CurrentVelocity,  // ExactSpring
												      const FVector TargetVector,     // ExactSpring
												      const FVector TargetVelocity,   // ExactSpring
												      FVector& OutVelocity,           // ExactSpring
												      const FVector PreviousResidual, // ContinuousNaive
													  FVector& DeltaResidual          // Lowpass
											)
{
	EasyDamp(DampParams, Input.Roll, DeltaSeconds, DampTime, Output.Roll, TemporalInput[0], SpatialInput[0], DampParams.SpringCoefficient[0], CurrentVelocity[0], TargetVector[0], TargetVelocity[0], DampParams.DampRatio[0], DampParams.HalfLife[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input.Pitch, DeltaSeconds, DampTime, Output.Pitch, TemporalInput[1], SpatialInput[1], DampParams.SpringCoefficient[1], CurrentVelocity[1], TargetVector[1], TargetVelocity[1], DampParams.DampRatio[1], DampParams.HalfLife[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input.Yaw, DeltaSeconds, DampTime, Output.Yaw, TemporalInput[2], SpatialInput[2], DampParams.SpringCoefficient[2], CurrentVelocity[2], TargetVector[2], TargetVelocity[2], DampParams.DampRatio[2], DampParams.HalfLife[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampVectorWithDifferentDampTime(const FDampParams DampParams,   // All methods
														  const FVector Input,            // All methods
														  const float DeltaSeconds,       // All methods
														  const FVector DampTime,         // All methods
														  FVector& Output,                // All methods
														  const FVector TemporalInput,    // Spring
														  const FVector SpatialInput,     // Spring
														  const FVector CurrentVelocity,  // ExactSpring
														  const FVector TargetVector,     // ExactSpring
														  const FVector TargetVelocity,   // ExactSpring
														  FVector& OutVelocity,           // ExactSpring
														  const FVector PreviousResidual, // ContinuousNaive
														  FVector& DeltaResidual          // Lowpass
												)
{
	EasyDamp(DampParams, Input[0], DeltaSeconds, DampTime[0], Output[0], TemporalInput[0], SpatialInput[0], DampParams.SpringCoefficient[0], CurrentVelocity[0], TargetVector[0], TargetVelocity[0], DampParams.DampRatio[0], DampParams.HalfLife[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input[1], DeltaSeconds, DampTime[1], Output[1], TemporalInput[1], SpatialInput[1], DampParams.SpringCoefficient[1], CurrentVelocity[1], TargetVector[1], TargetVelocity[1], DampParams.DampRatio[1], DampParams.HalfLife[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input[2], DeltaSeconds, DampTime[2], Output[2], TemporalInput[2], SpatialInput[2], DampParams.SpringCoefficient[2], CurrentVelocity[2], TargetVector[2], TargetVelocity[2], DampParams.DampRatio[2], DampParams.HalfLife[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDampRotatorWithDifferentDampTime(const FDampParams DampParams,  // All methods
														  const FRotator Input,           // All methods
														  const float DeltaSeconds,       // All methods
														  const FVector DampTime,         // All methods
														  FRotator& Output,               // All methods
														  const FVector TemporalInput,    // Spring
														  const FVector SpatialInput,     // Spring
														  const FVector CurrentVelocity,  // ExactSpring
														  const FVector TargetVector,     // ExactSpring
														  const FVector TargetVelocity,   // ExactSpring
														  FVector& OutVelocity,           // ExactSpring
														  const FVector PreviousResidual, // ContinuousNaive
														  FVector& DeltaResidual          // Lowpass
												)
{
	EasyDamp(DampParams, Input.Roll, DeltaSeconds, DampTime[0], Output.Roll, TemporalInput[0], SpatialInput[0], DampParams.SpringCoefficient[0], CurrentVelocity[0], TargetVector[0], TargetVelocity[0], DampParams.DampRatio[0], DampParams.HalfLife[0], OutVelocity[0], PreviousResidual[0], DeltaResidual[0]);
	EasyDamp(DampParams, Input.Pitch, DeltaSeconds, DampTime[1], Output.Pitch, TemporalInput[1], SpatialInput[1], DampParams.SpringCoefficient[1], CurrentVelocity[1], TargetVector[1], TargetVelocity[1], DampParams.DampRatio[1], DampParams.HalfLife[1], OutVelocity[1], PreviousResidual[1], DeltaResidual[1]);
	EasyDamp(DampParams, Input.Yaw, DeltaSeconds, DampTime[2], Output.Yaw, TemporalInput[2], SpatialInput[2], DampParams.SpringCoefficient[2], CurrentVelocity[2], TargetVector[2], TargetVelocity[2], DampParams.DampRatio[2], DampParams.HalfLife[2], OutVelocity[2], PreviousResidual[2], DeltaResidual[2]);
}

void UECameraLibrary::EasyDamp(const FDampParams DampParams,   // All methods
							   const float Input,              // All methods
							   const float DeltaSeconds,       // All methods
							   const float DampTime,           // All methods
							   float& Output,                  // All methods
							   const float TemporalInput,      // Spring
	                           const float SpatialInput,       // Spring
							   const float SpringCoefficient,  // Spring
							   const float CurrentVelocity,    // ExactSpring
							   const float TargetValue,        // ExactSpring
							   const float TargetVelocity,     // ExactSpring
							   const float DampRatio,          // ExactSpring
							   const float HalfLife,           // ExactSpring
							   float& OutVelocity,             // ExactSpring
							   const float PreviousResidual,   // ContinuousNaive
							   float& DeltaResidual            // Lowpass
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
			SpringDampValue(DampParams.MaxDeltaSeconds, SpringCoefficient, DampParams.SpringResidual, DeltaSeconds, TemporalInput, SpatialInput, Output);
		}
		break;
		case EDampMethod::ExactSpring:
		{
			ExactSpringDamperValue(0.0, CurrentVelocity, TargetValue, TargetVelocity, DampRatio, HalfLife, DeltaSeconds, Output, OutVelocity);
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

void UECameraLibrary::DamperValue(const FDampParams& DampParams, const float& DeltaSeconds, const float& Input, float DampTime, float& Output)
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

void UECameraLibrary::DamperVectorWithSameDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& Input, float DampTime, FVector& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.X, DampTime, Output.X);
	DamperValue(DampParams, DeltaSeconds, Input.Y, DampTime, Output.Y);
	DamperValue(DampParams, DeltaSeconds, Input.Z, DampTime, Output.Z);
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

void UECameraLibrary::DamperRotatorWithDifferentDampTime(const FDampParams& DampParams, const float& DeltaSeconds, const FRotator& Input, FVector DampTime, FRotator& Output)
{
	DamperValue(DampParams, DeltaSeconds, Input.Roll, DampTime.X, Output.Roll);
	DamperValue(DampParams, DeltaSeconds, Input.Pitch, DampTime.Y, Output.Pitch);
	DamperValue(DampParams, DeltaSeconds, Input.Yaw, DampTime.Z, Output.Yaw);
}

/** 
* @TODO: This function needs to be improved for more stable behaviour. 
*/
void UECameraLibrary::SpringDampVector(const FDampParams& DampParams, const float& DeltaSeconds, const FVector& TemporalInput, const FVector& SpatialInput, FVector& Output)
{
	FVector SpatialDelta = SpatialInput;
	FVector TemporalDelta = TemporalInput;

	FVector Acceleration;
	float RemainingTime = DeltaSeconds; 
	Output = FVector(0, 0, 0);
	while (RemainingTime > 1e-5)
	{
		const float DampTime = FMath::Min(DampParams.MaxDeltaSeconds, RemainingTime);
		Acceleration = DampParams.SpringCoefficient * SpatialDelta;
		TemporalDelta = (1 - DampParams.SpringResidual) * TemporalDelta + DampTime * DampTime * Acceleration;
		SpatialDelta -= TemporalDelta;
		Output += TemporalDelta;
		RemainingTime -= DampTime;
	}
}

void UECameraLibrary::SpringDampValue(const float& MaxDeltaSeconds, const float& SpringCoefficient, const float& SpringResidual, const float& DeltaSeconds, const float& TemporalInput, const float& SpatialInput, float& Output)
{
	float SpatialDelta = SpatialInput;
	float TemporalDelta = TemporalInput;

	float Acceleration;
	float RemainingTime = DeltaSeconds;
	Output = 0.0f;
	while (RemainingTime > 1e-5)
	{
		const float DampTime = FMath::Min(MaxDeltaSeconds, RemainingTime);
		Acceleration = SpringCoefficient * SpatialDelta;
		TemporalDelta = (1 - SpringResidual) * TemporalDelta + DampTime * DampTime * Acceleration;
		SpatialDelta -= TemporalDelta;
		Output += TemporalDelta;
		RemainingTime -= DampTime;
	}
}

void UECameraLibrary::ExactSpringDamperVector(const FVector& CurrentVector, const FVector& CurrentVelocity, const FVector& TargetVector, const FVector& TargetVelocity, FVector DampRatio, FVector HalfLife, const float& DeltaSeconds, FVector& OutVector, FVector& OutVelocity)
{
	ExactSpringDamperValue(CurrentVector[0], CurrentVelocity[0], TargetVector[0], TargetVelocity[0], DampRatio[0], HalfLife[0], DeltaSeconds, OutVector[0], OutVelocity[0]);
	ExactSpringDamperValue(CurrentVector[1], CurrentVelocity[1], TargetVector[1], TargetVelocity[1], DampRatio[1], HalfLife[1], DeltaSeconds, OutVector[1], OutVelocity[1]);
	ExactSpringDamperValue(CurrentVector[2], CurrentVelocity[2], TargetVector[2], TargetVelocity[2], DampRatio[2], HalfLife[2], DeltaSeconds, OutVector[2], OutVelocity[2]);
}

void UECameraLibrary::ExactSpringDamperValue(const float& CurrentValue, const float& CurrentVelocity, const float& TargetValue, const float& TargetVelocity, float DampRatio, float HalfLife, const float& DeltaSeconds, float& OutValue, float& OutVelocity)
{
	float G = TargetValue;
	float Q = TargetVelocity;
	float D = (4.0f * 0.69314718056f) / (HalfLife + 1e-8f);
	float S = FMath::Square<float>(D / (DampRatio * 2.0f));
	float C = G + (D * Q) / (S + 1e-8f);
	float Y = D / 2.0f;

	// Critically damped
	if (FMath::Abs(S - D * D / 4.0f) < 1e-5f)
	{
		float J0 = CurrentValue - C;
		float J1 = CurrentVelocity + J0 * Y;

		float YDt = Y * DeltaSeconds;
		float E = 1.0f / (1.0f + YDt + 0.48f * YDt * YDt + 0.235f * YDt * YDt * YDt);

		OutValue = J0 * E + DeltaSeconds * J1 * E + C;
		OutVelocity = -Y * J0 * E - Y * DeltaSeconds * J1 * E + J1 * E;
	}
	// Under damped
	else if (S - D * D / 4.0f > 0.0f)
	{
		float W = FMath::Sqrt(S - D * D / 4.0f);
		float J = FMath::Sqrt(FMath::Square<float>(CurrentVelocity + Y * (CurrentValue - C)) / (W * W + 1e-8f) + FMath::Square<float>(CurrentValue - C));
		float P = FastAtan((CurrentVelocity + (CurrentValue - C) * Y) / (-(CurrentValue - C) * W + 1e-8f));

		J = (CurrentValue - C) > 0.0f ? J : -J;

		float YDt = Y * DeltaSeconds;
		float E = 1.0f / (1.0f + YDt + 0.48f * YDt * YDt + 0.235f * YDt * YDt * YDt);

		OutValue = J * E * FMath::Cos(W * DeltaSeconds + P) + C;
		OutVelocity = -Y * J * E * FMath::Cos(W * DeltaSeconds + P) - W * J * E * FMath::Sin(W * DeltaSeconds + P);
	}
	// Over damped
	else if (S - D * D / 4.0f < 0.0f)
	{
		float Y0 = (D + FMath::Sqrt(D * D - 4.0f * S)) / 2.0f;
		float Y1 = (D - FMath::Sqrt(D * D - 4.0f * S)) / 2.0f;
		float J1 = (C * Y0 - CurrentValue * Y0 - CurrentVelocity) / (Y1 - Y0);
		float J0 = CurrentValue - J1 - C;

		float Y0Dt = Y0 * DeltaSeconds;
		float Y1Dt = Y1 * DeltaSeconds;
		float E0 = 1.0f / (1.0f + Y0Dt + 0.48f * Y0Dt * Y0Dt + 0.235f * Y0Dt * Y0Dt * Y0Dt);
		float E1 = 1.0f / (1.0f + Y1Dt + 0.48f * Y1Dt * Y1Dt + 0.235f * Y1Dt * Y1Dt * Y1Dt);

		OutValue = J0 * E0 + J1 * E1 + C;
		OutVelocity = -Y0 * J0 * E0 - Y1 * J1 * E1;
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

void UECameraLibrary::ContinuousDamperValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, float& Output)
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
		CoefficientTerm += FMath::Pow((double)Derivative, i - 1) * FMath::Pow((double)DeltaSeconds, i) / Factorials[i];
	}
	float DeltaResidual = VelocityTerm * CoefficientTerm;
	
	Output = Input - (PreviousResidual + DeltaResidual);
}

void UECameraLibrary::RestrictionDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const int& Power, const float& DeltaSeconds, float DampTime, float& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	if (FMath::Abs(Input) < 1e-4 || PreviousResidual / Input <= 0)
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
			Delta = FMath::Exp(Delta * FMath::Loge(1.0f + Tolerance) / Tolerance) - 1;
			Delta = Delta * FMath::Pow(Delta / Tolerance, Power);

			NewDeltaSeconds = Ratio <= 1.0f ? Alpha * (1.0f - Delta) : Alpha * (1.0f + Delta);
		}
	}

	Output = Input * (1.0f - FMath::Exp(lnResidual * NewDeltaSeconds / DampTime));
}

void UECameraLibrary::LowpassDampValue(const FDampParams& DampParams, const float& Input, const float& PreviousResidual, const float& DeltaSeconds, float DampTime, float& DeltaResidual, float& Output)
{
	if (DeltaSeconds <= 0) { Output = 0.0f; return; }
	if (DampTime <= 0) { Output = Input; return; }

	if (FMath::Abs(Input) < 1e-4 || PreviousResidual / Input <= 0)
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
	float T;
	DamperValue(FDampParams(), DeltaSeconds, 1, DampTime, T);
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
								AActor* ParentCamera                      // Parent camera
					)
{
	if (!CameraClass)
	{
		return nullptr;
	}
	/*
	AECameraBase* ActiveCamera = GetActiveCamera(WorldContextObject);
	if (IsValid(ActiveCamera) && ActiveCamera->GetClass() == CameraClass->GetOwnerClass() && !ActiveCamera->bIsTransitory && !bIsTransitory) 
	{
		return ActiveCamera;
	}
	*/
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
		ParentCamera
	);
}

AECameraBase* UECameraLibrary::CallAnimatedCamera(const UObject* WorldContextObject,            // World context object.
												  UAnimSequence* AnimToPlay,                    // Animation sequence to play.
												  FTransform RefCoordinate,                     // In which reference frame you want to play the camera animation.
												  AActor* RefCoordinateActor,					// In which actor's local space you want to play the camera animation.
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
		AnimatedCameraExtension->SetRefActor(RefCoordinateActor);
		AnimatedCameraExtension->SetRef(RefCoordinate);
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
	AECameraManager* Manager = Cast<AECameraManager>(UGameplayStatics::GetActorOfClass(World, AECameraManager::StaticClass()));
	if (Manager != nullptr) Manager->TerminateActiveCamera();
}

AECameraBase* UECameraLibrary::GetActiveCamera(const UObject* WorldContextObject)
{
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(WorldContextObject, AECameraManager::StaticClass());
	if (ManagerActor != nullptr)
	{
		return Cast<AECameraManager>(ManagerActor)->GetActiveCamera();
	}
	else return nullptr;
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

void UECameraLibrary::Generic_AccessCameraPropertyByName(UObject* OwnerObject, FName PropertyName, void* ValuePtr, FProperty* ValueProp, bool bSetter)
{
	if (OwnerObject)
	{
		FProperty* Property = FindFProperty<FProperty>(OwnerObject->GetClass(), PropertyName);

		if (Property && (Property->SameType(ValueProp)))
		{
			void* PropPtr = Property->ContainerPtrToValuePtr<void>(OwnerObject);

			if (bSetter)
			{
				Property->CopySingleValue(PropPtr, ValuePtr);
			}
			else
			{
				Property->CopySingleValue(ValuePtr, PropPtr);
			}
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("UECameraLibrary::Generic_GetCameraPropertyByName: Failed to find %s property from %s object"), *PropertyName.ToString(), *UKismetSystemLibrary::GetDisplayName(OwnerObject))
}