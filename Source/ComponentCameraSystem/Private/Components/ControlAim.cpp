// Copyright 2023 by Sulley. All Rights Reserved.

#include "Components/ControlAim.h"
#include "Components/OrbitFollow.h"
#include "Components/FramingFollow.h"
#include "Utils/ECameraLibrary.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputActionValue.h"

UControlAim::UControlAim()
{
	Stage = EStage::Aim;

	bEnhancedInput = false;
	AimAssist = FAimAssist();
	HorizontalHeading = EHeading::WorldForward;
	HorizontalHardForward = FVector(1, 0, 0);
	HorizontalRange = FVector2D(-180, 180);
	VerticalRange = FVector2D(-70, 70);
	bWrap = true;
	HorizontalSpeed = 1.0f;
	VerticalSpeed = 1.0f;
	HorizontalDamping = FVector2D(0.2f, 0.2f);
	VerticalDamping = FVector2D(0.2f, 0.2f);
	bSyncToController = false;
	bSyncYawToAimTarget = false;

	CachedMouseDeltaX = 0.0f;
	CachedMouseDeltaY = 0.0f;
	WaitElaspedTime = 0.0f;
	bInAimAssist = false;
	ElapsedUpdateTime = 10.0f;
	OffsetInAimAssist = FVector();
}

void UControlAim::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningSettingComponent()->GetPlayerController()->GetLocalPlayer());
}

void UControlAim::UpdateComponent_Implementation(float DeltaTime)
{
	GetMouseDelta();
	
	/** Resolve recentering. */
	if (ResolveRecentering(DeltaTime))
	{
		CachedMouseDeltaX = 0;
		CachedMouseDeltaY = 0;
		return;
	}

	/** Apply speed multiplier. */
	RawMouseDeltaX = RawMouseDeltaX * HorizontalSpeed;
	RawMouseDeltaY = RawMouseDeltaY * VerticalSpeed;

	/** Damp, constrain and set camera yaw. */
	float ResultDeltaX = CachedMouseDeltaX + GetDampedMouseDelta(RawMouseDeltaX, true, DeltaTime);
	float WrapYaw = ConstrainYaw(ResultDeltaX);
	ResultDeltaX += WrapYaw;

	/** Damp, constrain and set camera pitch. */
	float ResultDeltaY = CachedMouseDeltaY + GetDampedMouseDelta(RawMouseDeltaY, false, DeltaTime);
	ConstrainPitch(ResultDeltaY);

	/** Cache world and local follow position for rectification. */
	bool bRectifyPosition = IsValid(GetOwningSettingComponent()->GetFollowComponent()) && GetOwningSettingComponent()->GetFollowComponent()->IsA<UFramingFollow>();
	if (bRectifyPosition)
	{
		UFramingFollow* FramingFollow = Cast<UFramingFollow>(GetOwningSettingComponent()->GetFollowComponent());
		WorldFollowPosition = FramingFollow->GetFollowPosition();
		LocalFollowPosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), WorldFollowPosition);
	}
	
	/** If not in aim assist. */
	if (!bInAimAssist)
	{
		GetOwningActor()->AddActorWorldRotation(FRotator(0, ResultDeltaX, 0));
		GetOwningActor()->AddActorLocalRotation(FRotator(ResultDeltaY, 0, 0));
		
		if (bRectifyPosition)
		{
			FVector RectifiedFollowPosition = UKismetMathLibrary::TransformDirection(GetOwningActor()->GetActorTransform(), LocalFollowPosition) + GetOwningActor()->GetActorLocation();
			GetOwningActor()->AddActorWorldOffset(WorldFollowPosition - RectifiedFollowPosition);
		}
	}
	else
	{
		/** Pitch first, then yaw. */
		FVector2D ControlForce  = FVector2D(ResultDeltaY, ResultDeltaX);
		FVector2D MagneticForce = FVector2D();
		FVector2D FinalForce    = FVector2D();

		if (ActorInAimAssist)
		{
			FVector OffsetTargetPosition = UECameraLibrary::GetPositionWithLocalOffset(ActorInAimAssist, OffsetInAimAssist);
			FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(GetOwningActor()->GetActorLocation(), OffsetTargetPosition), GetOwningActor()->GetActorRotation());
			
			float lnResidual = FMath::Loge(0.1);
			float Multiplier = FMath::Exp(lnResidual * ScreenDistanceInAimAssist / AimAssist.MagneticRadius);
			MagneticForce = AimAssist.MagneticCoefficient * Multiplier * FVector2D(DeltaRotation.Pitch, DeltaRotation.Yaw);
		}

		FinalForce = ControlForce + MagneticForce;
		GetOwningActor()->AddActorWorldRotation(FRotator(0, FinalForce.Y, 0));
		GetOwningActor()->AddActorLocalRotation(FRotator(FinalForce.X, 0, 0));
	}

	/** Update cached delta. */
	CachedMouseDeltaX = ResultDeltaX;
	CachedMouseDeltaY = ResultDeltaY;

	/** Check if there exists an actor that will be in aim assist. */
	ElapsedUpdateTime += DeltaTime;
	bInAimAssist = CheckAimAssist();

	/** Sync with Controller. */
	if (bSyncToController)
	{
		GetOwningSettingComponent()->GetPlayerController()->SetControlRotation(GetOwningActor()->GetActorRotation());
	}

	/** Sync yaw to aim target. */
	if (bSyncYawToAimTarget)
	{
		AActor* Target = GetOwningSettingComponent()->GetAimTarget();
		if (IsValid(Target))
		{
			double DeltaYaw = GetOwningActor()->GetActorRotation().Yaw - Target->GetActorRotation().Yaw;

			/** Remap to [-180,180] */
			while (DeltaYaw >= 180)
			{
				DeltaYaw -= 360;
			}
			while (DeltaYaw <= -180)
			{
				DeltaYaw += 360;
			}

			float DampedDeltaYaw;
			UECameraLibrary::DamperValue(FDampParams(), DeltaTime, DeltaYaw, 0.5f, DampedDeltaYaw);
			Target->AddActorWorldRotation(FRotator(0, DampedDeltaYaw, 0));
		}
	}
}

void UControlAim::GetMouseDelta()
{
	/** Read from mouse input. */
	if (!bEnhancedInput)
	{
		GetOwningSettingComponent()->GetPlayerController()->GetInputMouseDelta(RawMouseDeltaX, RawMouseDeltaY);
	}
	/** Read from enhanced input. */
	else
	{
		bool bSuccess = false;
		if (Subsystem)
		{
			UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
			if (LookAction && PlayerInput)
			{
				FInputActionValue Value = PlayerInput->GetActionValue(LookAction);
				FVector2D LookAxisVector = Value.Get<FVector2D>();
				RawMouseDeltaX = LookAxisVector.X;
				RawMouseDeltaY = -LookAxisVector.Y;

				bSuccess = true;
			}
		}

		if (!bSuccess)
		{
			GetOwningSettingComponent()->GetPlayerController()->GetInputMouseDelta(RawMouseDeltaX, RawMouseDeltaY);
		}
	}
}

float UControlAim::GetDampedMouseDelta(const float& MouseDelta, bool bIsHorizontal, const float& DeltaTime)
{
	float DampTime;
	if (bIsHorizontal) DampTime = FMath::Abs(MouseDelta) < FMath::Abs(CachedMouseDeltaX) ? HorizontalDamping.Y : HorizontalDamping.X;
	else DampTime = FMath::Abs(MouseDelta) < FMath::Abs(CachedMouseDeltaY) ? VerticalDamping.Y : VerticalDamping.X;

	float Output = 0;
	UECameraLibrary::DamperValue(FDampParams(), DeltaTime, (bIsHorizontal ? MouseDelta - CachedMouseDeltaX : MouseDelta - CachedMouseDeltaY), DampTime, Output);
	return Output;
}

bool UControlAim::ResolveRecentering(const float& DeltaTime)
{
	if (RecenteringParams.bRecentering)
	{
		/** If input is not zero, return false. */
		if (RawMouseDeltaX != 0 || RawMouseDeltaX != 0)
		{
			WaitElaspedTime = 0;
			return false;
		}

		WaitElaspedTime += DeltaTime;
		if (WaitElaspedTime >= RecenteringParams.WaitTime)
		{
			FQuat TargetQuat = GetRecenteringTargetQuat();
			FQuat OutputQuat;
			UECameraLibrary::DamperQuaternion(GetOwningActor()->GetActorQuat(), TargetQuat, DeltaTime, RecenteringParams.RecenteringTime, OutputQuat);

			/** Set roll to 0, and apply to camera. */
			FRotator OutputRotation = OutputQuat.Rotator();
			OutputRotation.Roll = 0;
			GetOwningActor()->SetActorRotation(OutputRotation);
			return true;
		}
		else return false;
	}
	else return false;
}

FQuat UControlAim::GetRecenteringTargetQuat()
{
	FRotator TargetRotation = FRotator::ZeroRotator;
	FQuat TargetQuat = FQuat::Identity;

	/** Heading to WorldForward. */
	if (RecenteringParams.Heading == EHeading::WorldForward) 
		TargetRotation = FRotator::ZeroRotator;

	/** Heading to TargetForward. */
	else if (RecenteringParams.Heading == EHeading::TargetForward)
	{
		if (GetOwningSettingComponent()->GetFollowTarget() != nullptr)
			TargetRotation = GetOwningSettingComponent()->GetFollowTarget()->GetActorRotation();
		else
			TargetRotation = GetOwningActor()->GetActorRotation();
	}

	/** Heading to HardForward. */
	else if (RecenteringParams.Heading == EHeading::HardForward)
	{
		TargetRotation = RecenteringParams.HardForward.Rotation();
	}

	/** Heading to SoftForward. */
	else if (RecenteringParams.Heading == EHeading::SoftForward)
	{
		if (RecenteringParams.Source != nullptr && RecenteringParams.Destination != nullptr)
			TargetRotation = (RecenteringParams.Destination->GetActorLocation() - RecenteringParams.Source->GetActorLocation()).Rotation();
		else TargetRotation = GetOwningActor()->GetActorRotation();
	}

	/** Whether to reset pitch. */
	if (RecenteringParams.bResetPitch) TargetRotation.Pitch = 0;
	else TargetRotation.Pitch = GetOwningActor()->GetActorRotation().Pitch;

	TargetQuat = TargetRotation.Quaternion();
	return TargetQuat;
}

float UControlAim::ConvertYaw(float Yaw)
{
	float ConvertedYaw = Yaw;
	while (ConvertedYaw > 180) ConvertedYaw -= 360;
	while (ConvertedYaw < -180) ConvertedYaw += 360;
	return ConvertedYaw;
}

float UControlAim::ConstrainYaw(float& ResultDeltaX)
{
	float CenteringYaw = 0;
	if (HorizontalHeading == EHeading::WorldForward)
		CenteringYaw = 0;
	else if (HorizontalHeading == EHeading::TargetForward)
		CenteringYaw = GetOwningSettingComponent()->GetFollowTarget() != nullptr ? GetOwningSettingComponent()->GetFollowTarget()->GetActorRotation().Yaw : 0;
	else if (HorizontalHeading == EHeading::HardForward)
		CenteringYaw = HorizontalHardForward.Rotation().Yaw;
	else if (HorizontalHeading == EHeading::SoftForward)
		CenteringYaw = (HorizontalSource != nullptr && HorizontalDestination != nullptr)
						? (HorizontalDestination->GetActorLocation() - HorizontalSource->GetActorLocation()).Rotation().Yaw
						: 0;
	
	float YawBoundLeft = CenteringYaw + HorizontalRange.X;
	float YawBoundRight = CenteringYaw + HorizontalRange.Y;	
	float CameraYaw = GetOwningActor()->GetActorRotation().Yaw;
	float AddedCameraYaw = CameraYaw + ResultDeltaX;

	/** Whether the added camera yaw is within range. A little bit about math, but not hard. */
	if (FMath::FloorToInt((YawBoundRight - AddedCameraYaw) / 360) == FMath::CeilToInt((YawBoundLeft - AddedCameraYaw) / 360)) return 0.0f;
	/** If not within range, restrict yaw to bounds. Take bWrap into account. */
	if (ConvertYaw(AddedCameraYaw) > ConvertYaw(YawBoundRight))
	{
		if (!bWrap)
		{
			ResultDeltaX = ConvertYaw(YawBoundRight - CameraYaw);
			return 0.0f;
		}
		else return ConvertYaw(YawBoundLeft) + 360 - ConvertYaw(YawBoundRight);
	}
	if (ConvertYaw(AddedCameraYaw) < ConvertYaw(YawBoundLeft))
	{
		if (!bWrap)
		{
			ResultDeltaX = ConvertYaw(YawBoundLeft - CameraYaw);
			return 0.0f;
		}
		else return -(ConvertYaw(YawBoundLeft) + 360 - ConvertYaw(YawBoundRight));
	}
	return 0.0f;
}

void UControlAim::ConstrainPitch(float& ResultDeltaY)
{
	FRotator CameraRotation = GetOwningActor()->GetActorRotation();
	if (CameraRotation.Pitch + ResultDeltaY > VerticalRange.Y) ResultDeltaY = VerticalRange.Y - CameraRotation.Pitch;
	if (CameraRotation.Pitch + ResultDeltaY < VerticalRange.X) ResultDeltaY = VerticalRange.X - CameraRotation.Pitch;
}

bool UControlAim::CheckAimAssist()
{
	/** Out of performance consideration, you should turn off aim assist when not necessary, e.g., not in battle state. 
	 * A better approach is to dynamically update the OutActors when target actors are spawned / destroyed.
	 * But this requires modification to your specific actors. If you have performance requirements, you should change code here.
	 */
	if (AimAssist.bEnableAimAssist)
	{
		if (ElapsedUpdateTime >= AimAssist.TimeInterval)
		{
			ElapsedUpdateTime = 0.0f;
			OutActors.Empty();

			for (const FOffsetActorType& OffsetTargetType : AimAssist.TargetTypes)
			{
				TArray<AActor*> TempOutActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), OffsetTargetType.ActorType, TempOutActors);
				OutActors.Add(OffsetTargetType.Offset, TempOutActors);
			}
		}

		float ClosestDistance = 99999;
		for (auto& ActorsWithType : OutActors)
		{
			for (AActor* TargetActor : ActorsWithType.Value)
			{
				if (TargetActor == nullptr) continue;

				FVector RealPosition = UECameraLibrary::GetPositionWithLocalOffset(TargetActor, ActorsWithType.Key);
				FVector LocalSpacePosition = UECameraLibrary::GetLocalSpacePosition(GetOwningActor(), RealPosition);

				if (LocalSpacePosition.X > 0 && LocalSpacePosition.X <= AimAssist.MaxDistance)
				{
					LocalSpacePosition.X = 0;
					float Distance = LocalSpacePosition.Size();
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						ScreenDistanceInAimAssist = Distance;
						ActorInAimAssist = TargetActor;
						OffsetInAimAssist = ActorsWithType.Key;
					}
				}
			}
		}

		if (ScreenDistanceInAimAssist <= AimAssist.MagneticRadius)
		{
			return true;
		}
		else return false;
	}
	else return false;
}