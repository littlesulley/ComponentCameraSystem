// Copyright 2023 by Sulley. All Rights Reserved.


#include "Utils/ECameraDamper.h"

//////////////////////////
// UECameraDamperVector //
//////////////////////////

UECameraDamperVector::UECameraDamperVector()
{
	DamperX = CreateDefaultSubobject<UNaiveDamper>("Naive Damper X");
	DamperY = CreateDefaultSubobject<UNaiveDamper>("Naive Damper Y");
	DamperZ = CreateDefaultSubobject<UNaiveDamper>("Naive Damper Z");
}

void UECameraDamperVector::SetInput(const FVector& _Input)
{
	Input = _Input;

	if (IsValid(DamperX)) DamperX->SetInput(Input[0]);
	if (IsValid(DamperY)) DamperY->SetInput(Input[1]);
	if (IsValid(DamperZ)) DamperZ->SetInput(Input[2]);
}

void UECameraDamperVector::SetOutput(const FVector& _Output)
{
	Output = _Output;

	if (IsValid(DamperX)) DamperX->SetOutput(Output[0]);
	if (IsValid(DamperY)) DamperY->SetOutput(Output[1]);
	if (IsValid(DamperZ)) DamperZ->SetOutput(Output[2]);
}

FVector UECameraDamperVector::ApplyDamp(const float& DeltaTime)
{
	if (IsValid(DamperX)) Output[0] = DamperX->ApplyDamp(DeltaTime);
	if (IsValid(DamperY)) Output[1] = DamperY->ApplyDamp(DeltaTime);
	if (IsValid(DamperZ)) Output[2] = DamperZ->ApplyDamp(DeltaTime);

	return Output;
}

void UECameraDamperVector::PostApplyDamp()
{
	if (IsValid(DamperX)) DamperX->PostApplyDamp();
	if (IsValid(DamperY)) DamperY->PostApplyDamp();
	if (IsValid(DamperZ)) DamperZ->PostApplyDamp();
}

///////////////////////////
// UECameraDamperRotator //
///////////////////////////

UECameraDamperRotator::UECameraDamperRotator()
{
	DamperR = CreateDefaultSubobject<UNaiveDamper>("Naive Damper Roll");
	DamperP = CreateDefaultSubobject<UNaiveDamper>("Naive Damper Pitch");
	DamperY = CreateDefaultSubobject<UNaiveDamper>("Naive Damper Yaw");
}

void UECameraDamperRotator::SetInput(const FRotator& _Input)
{
	Input = _Input;

	if (IsValid(DamperR)) DamperR->SetInput(Input.Roll);
	if (IsValid(DamperP)) DamperP->SetInput(Input.Pitch);
	if (IsValid(DamperY)) DamperY->SetInput(Input.Yaw);
}

void UECameraDamperRotator::SetOutput(const FRotator& _Output)
{
	Output = _Output;

	if (IsValid(DamperR)) DamperR->SetOutput(Output.Roll);
	if (IsValid(DamperP)) DamperP->SetOutput(Output.Pitch);
	if (IsValid(DamperY)) DamperY->SetOutput(Output.Yaw);
}

FRotator UECameraDamperRotator::ApplyDamp(const float& DeltaTime)
{
	if (IsValid(DamperR)) Output.Roll  = DamperR->ApplyDamp(DeltaTime);
	if (IsValid(DamperP)) Output.Pitch = DamperP->ApplyDamp(DeltaTime);
	if (IsValid(DamperY)) Output.Yaw   = DamperY->ApplyDamp(DeltaTime);

	return Output;
}

void UECameraDamperRotator::PostApplyDamp()
{
	if (IsValid(DamperR)) DamperR->PostApplyDamp();
	if (IsValid(DamperP)) DamperP->PostApplyDamp();
	if (IsValid(DamperY)) DamperY->PostApplyDamp();
}

////////////////////////
// UNaiveDamper  //
////////////////////////

float UNaiveDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	if (DeltaTime <= 0) { Output = 0.0f; return Output; }
	if (DampTime <= 0) { Output = Input; return Output; }

	float lnResidual = FMath::Loge(Residual);
	Output = Input * (1.0f - FMath::Exp(lnResidual * DeltaTime / DampTime));
	return Output;
}

///////////////////////////
// USimulateDamper  //
///////////////////////////

float USimulateDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	if (DeltaTime <= 0) { Output = 0.0f; return Output; }
	if (DampTime <= 0) { Output = Input; return Output; }

	float lnResidual = FMath::Loge(Residual);
	float Ratio = FMath::Exp(lnResidual * DeltaTime / SimulateCount / DampTime);
	Output = Input - Input / SimulateCount * Ratio * (1.0f - FMath::Exp(lnResidual * DeltaTime / DampTime)) / (1.0f - Ratio);
	return Output;
}

//////////////////////////////////
// UContinuousNaiveDamper  //
//////////////////////////////////

float UContinuousNaiveDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	if (DeltaTime <= 0) { Output = 0.0f; return Output; }
	if (DampTime <= 0) { Output = Input; return Output; }

	float Derivative = FMath::Loge(Residual) / DampTime;
	float Speed = (Input - PreviousResidual) / DeltaTime;
	float VelocityTerm = PreviousResidual * Derivative + Speed;

	float CoefficientTerm = 0.0f;
	for (int i = 1; i <= Order; ++i)
	{
		CoefficientTerm += FMath::Pow((double)Derivative, i - 1) * FMath::Pow((double)DeltaTime, i) / Factorials.at(i);
	}
	float DeltaResidual = VelocityTerm * CoefficientTerm;
	Output = Input - (PreviousResidual + DeltaResidual);
	return Output;
}

void UContinuousNaiveDamper::PostApplyDamp_Implementation()
{
	PreviousResidual = Input - Output;
}

////////////////////////////////
// URestrictNaiveDamper  //
////////////////////////////////

float URestrictNaiveDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	if (DeltaTime <= 0) { Output = 0.0f; return Output; }
	if (DampTime <= 0) { Output = Input; return Output; }

	float lnResidual = FMath::Loge(Residual);
	float Alpha = FMath::Loge(PreviousResidual / Input) * DampTime / lnResidual;
	float Ratio = DeltaTime / Alpha;
	float NewDeltaSeconds = DeltaTime;

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
	return Output;
}

void URestrictNaiveDamper::PostApplyDamp_Implementation()
{
	PreviousResidual = Input - Output;
}

///////////////////////////////
// ULowpassNaiveDamper  //
///////////////////////////////

float ULowpassNaiveDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	if (DeltaTime <= 0) { Output = 0.0f; return Output; }
	if (DampTime <= 0) { Output = Input; return Output; }

	float lnResidual = FMath::Loge(Residual);
	float CurrentResidual = Input * FMath::Exp(lnResidual * DeltaTime / DampTime);
	float CurrentDeltaResidual = CurrentResidual - PreviousResidual;

	float Alpha = FMath::Loge(PreviousResidual / Input) * DampTime / lnResidual;
	float Ratio = DeltaTime / Alpha;

	if (Ratio >= 1.0f - Tolerance && Ratio <= 1.0f + Tolerance)
	{
		DeltaResidual = (1 - Beta) * DeltaResidual + Beta * CurrentDeltaResidual;
		Output = Input - (DeltaResidual + PreviousResidual);
	}
	else
	{
		Output = Input - CurrentResidual;
	}
	return Output;
}

void ULowpassNaiveDamper::PostApplyDamp_Implementation()
{
	PreviousResidual = Input - Output;
}

/////////////////////////
// USpringDamper  //
/////////////////////////

float USpringDamper::ApplyDamp_Implementation(const float& DeltaTime)
{
	// Underdamped
	if (DampRatio < 1)
	{
		float SqrtOfOneMinusSquare = FMath::Sqrt(1.0 - DampRatio * DampRatio);
		float OmegaDotSqrt = Frequency * SqrtOfOneMinusSquare;
		float Inner = OmegaDotSqrt * DeltaTime;
		float OmegaDotZeta = Frequency * DampRatio;
		float Cosine = FMath::Cos(Inner);
		float Sine = FMath::Sin(Inner);

		float C1 = Input;
		float C2 = (Velocity + OmegaDotZeta * Input) / (OmegaDotSqrt + 1e-5f);
		float Decay = FMath::Exp(-OmegaDotZeta * DeltaTime);
		float X = C1 * Decay * Cosine + C2 * Decay * Sine;
		Velocity = -OmegaDotZeta * X + (Velocity + OmegaDotZeta * Input) * Decay * Cosine - Input * OmegaDotSqrt * Decay * Sine;
		Output = Input - X;
	}
	// Critically damped
	else if (DampRatio == 1)
	{
		float X = Input * FMath::Exp(-Frequency * DeltaTime);
		Velocity = -Frequency * X;
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

		float C1 = (-Velocity / Frequency - ZetaMinusSqrt * Input) / (2.0 * SqrtOfSquareMinusOne + 1e-5f);
		float C2 = Input - C1;
		float T1 = C1 * FMath::Exp(DeltaTime * NegOmegaDotPlus);
		float T2 = C2 * FMath::Exp(DeltaTime * NegOmegaDotMinus);
		float X = T1 + T2;
		Velocity = NegOmegaDotPlus * T1 + NegOmegaDotMinus * T2;
		Output = Input - X;
	}
	// Undamped
	else
	{
		float Cosine = FMath::Cos(Frequency * DeltaTime);
		float Sine = FMath::Sin(Frequency * DeltaTime);
		float X = Velocity / Frequency * Sine + Input * Cosine;
		Velocity = Velocity * Cosine - Frequency * Input * Sine;
		Output = Input - X;
	}

	return Output;
}