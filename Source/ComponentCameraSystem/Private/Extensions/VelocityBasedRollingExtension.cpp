// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/VelocityBasedRollingExtension.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ERollDelayAction.h"

UVelocityBasedRollingExtension::UVelocityBasedRollingExtension()
{
	// Should be applied at the last stage of the camera pipeline.
	Stage = EStage::Finalize;

	bInverse = false;
	MaxRoll = 10.f;
	MinVelocity = 200.f;
	RollWaitTime = 0.5f;
	RestoreWaitTime = 0.2f;
	RollScheme = ERollScheme::Constant;
	RollSpeed = 45.f;
	RollSpeedRatio = 0.01f;
	RollDamping = 1.f;
	RestoreSpeed = 100.f;
	RestoreDamping = 1.f;

	bRolling = false;
	ElapsedRollWaitTime = 0.f;
	ElapsedRestoreWaitTime = 0.f;
	CurrentRoll = 0.f;

	InitRollAndRestoreMultiplierCurve();
}

void UVelocityBasedRollingExtension::UpdateComponent_Implementation(float DeltaTime)
{
	AActor* FollowTarget = nullptr;

	if (GetOwningSettingComponent()->GetFollowTarget() != nullptr)
	{
		FollowTarget = GetOwningSettingComponent()->GetFollowTarget();
	}

	if (FollowTarget)
	{
		// When RollScheme is Manual, roll is fully controlled by you.
		if (RollScheme == ERollScheme::Manual)
		{
			if (bRolling)
			{
				MannualElapsedTime += DeltaTime;

				CurrentRoll = UKismetMathLibrary::Ease(MannualOriginRoll, MannualTargetRoll, MannualElapsedTime / MannualDuration, MannualBlendFunc, MannualBlendExp);

				if (MannualElapsedTime >= MannualDuration)
				{
					bRolling = false;
				}
			}
		}
		else
		{
			FVector WorldVelocity = FollowTarget->GetVelocity();
			FVector LocalVelocity = UECameraLibrary::GetLocalSpacePositionWithVectors(
				FVector::ZeroVector,
				GetOwningActor()->GetActorForwardVector(),
				GetOwningActor()->GetActorRightVector(),
				GetOwningActor()->GetActorUpVector(),
				WorldVelocity
			);

			float CurrentVelocity = LocalVelocity.Y;

			if (FMath::Abs(CurrentVelocity) > MinVelocity)
			{
				// If in rolling, immediately add to roll.
				if (bRolling)
				{
					AddRoll(DeltaTime, CurrentVelocity);
				}
				// Otherwise, check elapsed wait time.
				else
				{
					ElapsedRollWaitTime += DeltaTime;

					// If pass, start rolling.
					if (ElapsedRollWaitTime >= RollWaitTime)
					{
						bRolling = true;
						AddRoll(DeltaTime, CurrentVelocity);
					}
				}
			}
			else if (bRolling)
			{
				// If roll is already close to zero, reset all cached variables.
				// @TODO: should better deal with RestoreRoll and AddRoll.
				if (!ResetWhenRollIsSmall())
				{
					ElapsedRestoreWaitTime += DeltaTime;

					if (ElapsedRestoreWaitTime >= RestoreWaitTime)
					{
						RestoreRoll(DeltaTime);
					}
					else
					{
						RemainRoll();
					}
				}
			}
			else
			{
				ElapsedRollWaitTime = 0.0f;
				ElapsedRestoreWaitTime = 0.0f;
			}
		}

		GetOwningActor()->AddActorLocalRotation(FRotator(0, 0, CurrentRoll));
	}
}

void UVelocityBasedRollingExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	bRolling = false;
	ElapsedRollWaitTime = 0.f;
	ElapsedRestoreWaitTime = 0.f;
	CurrentRoll = 0.f;
}

void UVelocityBasedRollingExtension::BindToOnPreTickComponent()
{
	FRotator CurrentRotation = GetOwningActor()->GetActorRotation();
	GetOwningActor()->SetActorRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, 0));
}

void UVelocityBasedRollingExtension::InitRollAndRestoreMultiplierCurve()
{
	RollMultiplierCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	RollMultiplierCurve.EditorCurveData.AddKey(1.0f, 1.0f);
	RestoreMultiplierCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	RestoreMultiplierCurve.EditorCurveData.AddKey(1.0f, 1.0f);
}

void UVelocityBasedRollingExtension::AddRoll(const float& DeltaTime, const float& CurrentVelocity)
{
	float RollAddition = 0.f;

	if (RollScheme == ERollScheme::Constant)
	{
		RollAddition = FMath::Sign(CurrentVelocity) * RollSpeed * DeltaTime;
	}
	else if (RollScheme == ERollScheme::ProportionalToVelocity)
	{
		RollAddition = RollSpeedRatio * CurrentVelocity;
	}

	if (bInverse)
	{
		RollAddition *= -1;
	}
	
	RollAddition *= RollMultiplierCurve.GetRichCurveConst()->Eval(CurrentRoll);

	float ExpectedRoll = FMath::Clamp(CurrentRoll + RollAddition, -MaxRoll, MaxRoll);
	float DeltaRoll = ExpectedRoll - CurrentRoll;

	double DampedDeltaRoll;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaRoll, RollDamping, DampedDeltaRoll);

	CurrentRoll += DampedDeltaRoll;
}

void UVelocityBasedRollingExtension::RemainRoll()
{
	// Do nothing, will always reset to CurrentRoll when tick ends.
}

void UVelocityBasedRollingExtension::RestoreRoll(const float& DeltaTime)
{
	float RollAddition = RestoreSpeed * DeltaTime * RestoreMultiplierCurve.GetRichCurveConst()->Eval(CurrentRoll);
	float ExpectedRoll = CurrentRoll > 0 ? FMath::Max(0, CurrentRoll - RollAddition) : FMath::Min(0, CurrentRoll + RollAddition);

	float DeltaRoll = ExpectedRoll - CurrentRoll;
	double DampedDeltaRoll;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaRoll, RestoreDamping, DampedDeltaRoll);

	CurrentRoll += DampedDeltaRoll;
}

bool UVelocityBasedRollingExtension::ResetWhenRollIsSmall()
{
	bool bSmallEnough = FMath::Abs(CurrentRoll) < 0.1;

	if (bSmallEnough)
	{
		bRolling = false;
		CurrentRoll = 0.f;
		ElapsedRollWaitTime = 0.f;
		ElapsedRestoreWaitTime = 0.f;
	}

	return bSmallEnough;
}


void UVelocityBasedRollingExtension::AsyncStartRoll(float TargetRoll, float Duration, TEnumAsByte<EEasingFunc::Type> BlendFunc, float BlendExp, struct FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GetWorld())
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

		if (LatentActionManager.FindExistingAction<FERollDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, 
				new FERollDelayAction(
					GetOwningActor(),
					this,
					CurrentRoll, 
					TargetRoll, 
					Duration, 
					BlendFunc,
					BlendExp,
					LatentInfo)
			);
		}

		Action = LatentActionManager.FindExistingAction<FERollDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	}
	else
	{
		Action = nullptr;
	}
}

void UVelocityBasedRollingExtension::AsyncPauseRoll(bool bSetPause)
{
	if (Action)
	{
		Action->SetPause(bSetPause);
	}
}

void UVelocityBasedRollingExtension::StartRoll(float TargetRoll, float Duration, TEnumAsByte<EEasingFunc::Type> BlendFunc, float BlendExp)
{
	MannualOriginRoll = CurrentRoll;
	MannualTargetRoll = TargetRoll;
	MannualElapsedTime = 0.0f;
	MannualDuration = Duration;
	MannualBlendFunc = BlendFunc;
	MannualBlendExp = BlendExp;

	bRolling = true;
}

FVector UVelocityBasedRollingExtension::GetFollowTargetVelocity(AActor*& FollowTarget)
{
	if (GetOwningSettingComponent()->GetFollowTarget() != nullptr)
	{
		FollowTarget = GetOwningSettingComponent()->GetFollowTarget();
		return FollowTarget->GetVelocity();
	}
	else
	{
		FollowTarget = nullptr;
		return FVector::ZeroVector;
	}
}