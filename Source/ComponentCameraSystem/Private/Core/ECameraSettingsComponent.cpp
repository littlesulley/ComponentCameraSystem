// Copyright 2023 by Sulley. All Rights Reserved.

#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSubsystem.h"
#include "Core/ECameraComponentBase.h"
#include "Components/ECameraComponentAim.h"
#include "Components/ECameraComponentFollow.h"
#include "Components/HardLockAim.h"
#include "Components/ControlAim.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Extensions/KeyframeExtension.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/EnumRange.h"

UECameraSettingsComponent::UECameraSettingsComponent()
{
	bForceActive = false;

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UECameraSettingsComponent::OnRegister()
{
	Super::OnRegister();
	InitializeECameraComponents();
}

void UECameraSettingsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UECameraSettingsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsActive())
	{
		OnPreTickComponent.Broadcast();
		for (EStage Stage : TEnumRange<EStage>())
		{
			// In the order of Follow -> Aim -> Extension.
			if (FollowComponent && FollowComponent->GetStage() == Stage)
			{
				FollowComponent->UpdateComponent(DeltaTime);
			}
			if (AimComponent && AimComponent->GetStage() == Stage)
			{
				AimComponent->UpdateComponent(DeltaTime);
			}
			for (UECameraExtensionBase* Extension : Extensions)
			{
				if (Extension && Extension->GetStage() == Stage)
				{
					Extension->UpdateComponent(DeltaTime);
				}
			}
		}
		OnPostTickComponent.Broadcast();
	}
}

APawn* UECameraSettingsComponent::GetOwningPawn() const
{
	return Cast<APawn>(GetOwner());
}

AECameraBase* UECameraSettingsComponent::GetOwningCamera() const
{
	return Cast<AECameraBase>(GetOwner());
}

APlayerController* UECameraSettingsComponent::GetPlayerController() const
{
	return UGameplayStatics::GetPlayerController(this, 0);
}

APlayerCameraManager* UECameraSettingsComponent::GetPlayerCameraManager() const
{
	APlayerController* PlayerController = GetPlayerController();
	return PlayerController != nullptr ? PlayerController->PlayerCameraManager : nullptr;
}

bool UECameraSettingsComponent::GetForceActive() const
{
	return bForceActive;
}

void UECameraSettingsComponent::SetForceActive(bool InForceActive)
{
	bForceActive = InForceActive;
}

bool UECameraSettingsComponent::IsActive() const
{
	if (bForceActive)
	{
		return true;
	}

	AActor* Owner = GetOwner();

	if (APlayerController* PlayerController = GetPlayerController())
	{
		return Owner == PlayerController->GetViewTarget();
	}

	return false;
}

AActor* UECameraSettingsComponent::SetFollowTarget(AActor* NewFollowTarget)
{
	FollowTarget = NewFollowTarget;

	if (IsValid(FollowComponent))
	{
		FollowComponent->SetFollowTarget(NewFollowTarget);
	}

	return NewFollowTarget;
}

FName UECameraSettingsComponent::SetFollowSocket(FName NewFollowSocket)
{
	if (IsValid(FollowComponent))
	{
		return FollowComponent->SetFollowSocket(NewFollowSocket);
	}

	return NAME_None;
}

USceneComponent* UECameraSettingsComponent::SetFollowSceneComponent(USceneComponent* NewSceneComponent)
{
	if (IsValid(FollowComponent))
	{
		return FollowComponent->SetSceneComponent(NewSceneComponent);
	}

	return nullptr;
}

AActor* UECameraSettingsComponent::GetFollowTarget() const
{
	return FollowTarget;
}

UECameraComponentFollow* UECameraSettingsComponent::GetFollowComponent() const
{
	return FollowComponent;
}

void UECameraSettingsComponent::SetFollowComponent(UECameraComponentFollow* InFollowComponent)
{
	if (IsValid(InFollowComponent))
	{
		FollowComponent = InFollowComponent;
		FollowComponent->SetFollowTarget(FollowTarget);
		InitializeECameraComponent(FollowComponent);
	}
}

AActor* UECameraSettingsComponent::SetAimTarget(AActor* NewAimTarget)
{
	AimTarget = NewAimTarget;

	if (IsValid(AimComponent))
	{
		AimComponent->SetAimTarget(NewAimTarget);
	}

	return NewAimTarget;
}

FName UECameraSettingsComponent::SetAimSocket(FName NewAimSocket)
{
	if (IsValid(AimComponent))
	{
		return AimComponent->SetAimSocket(NewAimSocket);
	}

	return NAME_None;
}

USceneComponent* UECameraSettingsComponent::SetAimSceneComponent(USceneComponent* NewSceneComponent)
{
	if (IsValid(AimComponent))
	{
		return AimComponent->SetSceneComponent(NewSceneComponent);
	}

	return nullptr;
}

UECameraComponentFollow* UECameraSettingsComponent::GetFollowComponentOfClass(TSubclassOf<UECameraComponentFollow> FollowClass)
{
	return FollowComponent;
}

UECameraComponentAim* UECameraSettingsComponent::GetAimComponentOfClass(TSubclassOf<UECameraComponentAim> AimClass)
{
	return AimComponent;
}

UECameraExtensionBase* UECameraSettingsComponent::GetExtensionOfClass(TSubclassOf<UECameraExtensionBase> ExtensionClass)
{
	UECameraExtensionBase* Result = nullptr;

	for (UECameraExtensionBase* Extension : Extensions)
	{
		if (IsValid(Extension) && Extension->IsA(ExtensionClass->GetOwnerClass()))
		{
			Result = Extension;
			break;
		}
	}

	return Result;
}

void UECameraSettingsComponent::AddExtension(UECameraExtensionBase* InExtension)
{
	if (IsValid(InExtension))
	{
		for (UECameraExtensionBase* Extension : Extensions)
		{
			if (Extension == InExtension)
			{
				return;
			}
		}

		Extensions.Add(InExtension);
		InitializeECameraComponent(InExtension);
	}
}

AActor* UECameraSettingsComponent::GetAimTarget() const
{
	return AimTarget;
}

void UECameraSettingsComponent::SetAimComponent(UECameraComponentAim* InAimComponent)
{
	if (IsValid(InAimComponent))
	{
		AimComponent = InAimComponent;
		AimComponent->SetAimTarget(AimTarget);
		InitializeECameraComponent(AimComponent);
	}
}

UECameraComponentAim* UECameraSettingsComponent::GetAimComponent() const
{
	return AimComponent;
}

TArray<UECameraExtensionBase*> UECameraSettingsComponent::GetExtensions() const
{
	return Extensions;
}

void UECameraSettingsComponent::InitializeECameraComponents()
{
	if (FollowComponent != nullptr)
	{
		InitializeECameraComponent(FollowComponent);
		FollowTarget = FollowComponent->GetFollowTarget();
	}
	if (AimComponent != nullptr)
	{
		InitializeECameraComponent(AimComponent);
		AimTarget = AimComponent->GetAimTarget();
	}
	for (UECameraExtensionBase* Extension : Extensions)
		if (Extension != nullptr) 
			InitializeECameraComponent(Extension);

	/** Special case: when you are using HardLockAim and uses the follow target's rotation, TickGroup should be set to TG_PrePhysics.
	 *  This is because when you enable character's OrientRotationToMovement, the tick order would be CharacterLocation -> CameraRotation -> CharacterRotation,
	 *	which creates strong camera jitter. Settings to TG_PrePhysics defers camera component to the next frame to mitigate this problem.
	 */
	if (AimComponent != nullptr && AimComponent->IsA<UHardLockAim>() && AimTarget == FollowTarget)
		PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UECameraSettingsComponent::InitializeECameraComponent(UECameraComponentBase* Component)
{
	Component->SetOwningActor(GetOwner());
	Component->SetOwningCamera(GetOwningCamera());
	Component->SetOwningSettingComponent(this);

	/** Bind delegates. */
	OnPreTickComponent.AddUObject(Component, &UECameraComponentBase::BindToOnPreTickComponent);
	OnPostTickComponent.AddUObject(Component, &UECameraComponentBase::BindToOnPostTickComponent);
	OnBecomeViewTarget.AddUObject(Component, &UECameraComponentBase::BindToOnBecomeViewTarget);
	OnEndViewTarget.AddUObject(Component, &UECameraComponentBase::BindToOnEndViewTarget);
}

void UECameraSettingsComponent::BecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	/** If preserve state, reset the incoming camera as the state the current camera. */
	if (bPreserveState)
	{
		GetOwner()->SetActorLocation(GetPlayerController()->PlayerCameraManager->GetCameraLocation());
		/** Discard roll. */
		FRotator Rotation = GetPlayerController()->PlayerCameraManager->GetCameraRotation();
		GetOwner()->SetActorRotation(FRotator(Rotation.Pitch, Rotation.Yaw, 0));
	}
	/** Different components have different OnBecomeViewTarget behaviours. */
	if (FollowComponent) FollowComponent->ResetOnBecomeViewTarget(PC, bPreserveState);
	if (AimComponent) AimComponent->ResetOnBecomeViewTarget(PC, bPreserveState);
	for (UECameraExtensionBase* Extension : Extensions)
		if (Extension) Extension->ResetOnBecomeViewTarget(PC, bPreserveState);

	/** User customized OnBecomeViewTarget functions. */
	OnBecomeViewTarget.Broadcast(PC);
}

void UECameraSettingsComponent::EndViewTarget(APlayerController* PC)
{
	/** User customized OnEndViewTarget functions. */
	OnEndViewTarget.Broadcast(PC);

	/** Different components have different OnEndViewTarget behaviours. */
	if (FollowComponent) FollowComponent->ResetOnEndViewTarget(PC);
	if (AimComponent) AimComponent->ResetOnEndViewTarget(PC);
	for (UECameraExtensionBase* Extension : Extensions)
		if (Extension) Extension->ResetOnEndViewTarget(PC);
}

#if WITH_EDITOR
void UECameraSettingsComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetName());
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UECameraSettingsComponent, FollowComponent)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UECameraSettingsComponent, AimComponent)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UECameraSettingsComponent, Extensions))
		{
			InitializeECameraComponents();
		}
	}
}
#endif