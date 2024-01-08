// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extensions/VelocityBasedRollingExtension.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "Kismet/KismetMathLibrary.h"

/** Basically copied from DelayAction.h. */
class FERollDelayAction : public FPendingLatentAction
{
public:
	TWeakObjectPtr<AActor> Camera;
	TWeakObjectPtr<UVelocityBasedRollingExtension> Extension;
	float OriginRoll;
	float TargetRoll;
	float Duration;
	float ElapsedTime;
	TEnumAsByte<EEasingFunc::Type> BlendFunc;
	float BlendExp;
	bool bPaused;

	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FERollDelayAction(AActor* InCamera, UVelocityBasedRollingExtension* InExtension, float InOriginRoll, float InTargetRoll, float InDuration, TEnumAsByte<EEasingFunc::Type> InBlendFunc, float InBlendExp, const FLatentActionInfo& LatentInfo)
		: Camera (InCamera)
		, Extension (InExtension)
		, OriginRoll (InOriginRoll)
		, TargetRoll (InTargetRoll)
		, Duration (InDuration)
		, ElapsedTime (0.0f)
		, BlendFunc (InBlendFunc)
		, BlendExp (InBlendExp)
		, bPaused(false)
		, ExecutionFunction (LatentInfo.ExecutionFunction)
		, OutputLink (LatentInfo.Linkage)
		, CallbackTarget (LatentInfo.CallbackTarget)
	{ }

	// Return true when the action is completed
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (!bPaused)
		{
			ElapsedTime += Response.ElapsedTime();
		}

		if (Camera.IsValid() && Extension.IsValid())
		{
			Extension->CurrentRoll = UKismetMathLibrary::Ease(OriginRoll, TargetRoll, ElapsedTime / Duration, BlendFunc, BlendExp);
		}

		Response.FinishAndTriggerIf(ElapsedTime >= Duration, ExecutionFunction, OutputLink, CallbackTarget);
	}

	UFUNCTION(BlueprintCallable, Category = "VelocityBasedRollingExtension")
	void SetPause(bool bInPause)
	{
		bPaused = bInPause;
	}


#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(3)
			.SetMaximumFractionalDigits(3);
		return FText::Format(NSLOCTEXT("RollDelayAction", "DelayActionTimeFmt", "Delay ({0} seconds elapsed)"), FText::AsNumber(ElapsedTime, &DelayTimeFormatOptions)).ToString();
	}
#endif
};