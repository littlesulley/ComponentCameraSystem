// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/ECameraModifier_CameraShake.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "DefaultCameraShakeBase.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "DisplayDebugHelpers.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"


UECameraModifier_CameraShake::UECameraModifier_CameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }

UCameraShakeBase* UECameraModifier_CameraShake::AddCameraShakeWave(UCameraShakePattern* InPattern, const FAddCameraShakeParams& Params, bool bSingleInstance)
{
	TSubclassOf<UCameraShakeBase> ShakeClass = UDefaultCameraShakeBase::StaticClass();
	if (ShakeClass != nullptr)
	{
		float Scale = Params.Scale;
		const UCameraShakeSourceComponent* SourceComponent = Params.SourceComponent;
		const bool bIsCustomInitialized = Params.Initializer.IsBound();

		// Adjust for splitscreen
		if (CameraOwner != nullptr && CameraOwner->PCOwner != nullptr)
		{
			const ULocalPlayer* LocalPlayer = CameraOwner->PCOwner->GetLocalPlayer();
			if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr)
			{
				if (LocalPlayer->ViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None)
				{
					Scale *= SplitScreenShakeScale;
				}
			}
		}

		UCameraShakeBase const* const ShakeCDO = GetDefault<UCameraShakeBase>(ShakeClass);
		const bool bIsSingleInstance = ShakeCDO && bSingleInstance;
		if (bIsSingleInstance)
		{
			// Look for existing instance of same class
			for (FActiveCameraShakeInfo& ShakeInfo : ActiveShakes)
			{
				UCameraShakeBase* ShakeInst = ShakeInfo.ShakeInstance;
				if (ShakeInst && (ShakeClass == ShakeInst->GetClass()))
				{
					if (!ShakeInfo.bIsCustomInitialized && !bIsCustomInitialized)
					{
						// Just restart the existing shake, possibly at the new location.
						// Warning: if the shake source changes, this would "teleport" the shake, which might create a visual
						// artifact, if the user didn't intend to do this.
						ShakeInfo.ShakeSource = SourceComponent;
						// Set shake pattern.
						ShakeInst->SetRootShakePattern(InPattern);
						ShakeInst->StartShake(CameraOwner, Scale, Params.PlaySpace, Params.UserPlaySpaceRot);
						return ShakeInst;
					}
					else
					{
						// If either the old or new shake are custom initialized, we can't
						// reliably restart the existing shake and expect it to be the same as what the caller wants. 
						// So we forcibly stop the existing shake immediately and will create a brand new one.
						ShakeInst->StopShake(true);
						ShakeInst->TeardownShake();
						// Discard it right away so the spot is free in the active shakes array.
						ShakeInfo.ShakeInstance = nullptr;
					}
				}
			}
		}

		// Try to find a shake in the expired pool
		UCameraShakeBase* NewInst = ReclaimShakeFromExpiredPool(ShakeClass);

		// No old shakes, create a new one
		if (NewInst == nullptr)
		{
			NewInst = NewObject<UCameraShakeBase>(this, ShakeClass);
		}

		if (NewInst)
		{
			// Set shake pattern.
			NewInst->SetRootShakePattern(InPattern);

			// Custom initialization if necessary.
			if (bIsCustomInitialized)
			{
				Params.Initializer.Execute(NewInst);
			}

			// Initialize new shake and add it to the list of active shakes
			NewInst->StartShake(CameraOwner, Scale, Params.PlaySpace, Params.UserPlaySpaceRot);

			// Look for nulls in the array to replace first -- keeps the array compact
			bool bReplacedNull = false;
			for (int32 Idx = 0; Idx < ActiveShakes.Num(); ++Idx)
			{
				FActiveCameraShakeInfo& ShakeInfo = ActiveShakes[Idx];
				if (ShakeInfo.ShakeInstance == nullptr)
				{
					ShakeInfo.ShakeInstance = NewInst;
					ShakeInfo.ShakeSource = SourceComponent;
					ShakeInfo.bIsCustomInitialized = bIsCustomInitialized;
					bReplacedNull = true;

#if UE_ENABLE_DEBUG_DRAWING
					AddCameraShakeDebugData(ShakeInfo);
#endif
				}
			}

			// no holes, extend the array
			if (bReplacedNull == false)
			{
				FActiveCameraShakeInfo ShakeInfo;
				ShakeInfo.ShakeInstance = NewInst;
				ShakeInfo.ShakeSource = SourceComponent;
				ShakeInfo.bIsCustomInitialized = bIsCustomInitialized;
				ActiveShakes.Emplace(ShakeInfo);

#if UE_ENABLE_DEBUG_DRAWING
				AddCameraShakeDebugData(ActiveShakes.Last());
#endif
			}
		}

		return NewInst;
	}

	return nullptr;
}

UCameraShakeBase* UECameraModifier_CameraShake::AddCameraShakePerlin(UCameraShakePattern* InPattern, const FAddCameraShakeParams& Params, bool bSingleInstance)
{
	return AddCameraShakeWave(InPattern, Params, bSingleInstance);
}