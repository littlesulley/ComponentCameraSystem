// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/KeyframeExtension.h"
#include "ActorSequenceComponent.h"
#include "ActorSequence.h"
#include "ActorSequencePlayer.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/ECameraLibrary.h"
#include "Core/ECameraManager.h"

UKeyframeExtension::UKeyframeExtension()
{
	PCMGParams = FPCMGParams();
	LocationOffset = FVector();
}

void UKeyframeExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (SequencePlayer)
	{
		/** Use the internal SetPlaybackPosition function to immediately set play back position.  */
		SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(ElapsedFrames, EUpdatePositionMethod::Play));

		/** Apply location override. */
		if (LocationOverride.IsValid())
		{
			FVector NewLocation = UECameraLibrary::GetPositionWithLocalOffset(LocationOverride.Get(), GetOwningCamera()->GetCameraComponent()->GetRelativeLocation() + LocationOffset);
			GetOwningCamera()->GetCameraComponent()->SetWorldLocation(NewLocation);
		}

		/** Apply rotation override. */
		if (RotationOverride.IsValid())
		{
			FRotator NewRotator = RotationOverride.Get()->GetActorRotation() + GetOwningCamera()->GetCameraComponent()->GetRelativeRotation();
			GetOwningCamera()->GetCameraComponent()->SetWorldRotation(NewRotator);
		}

		/** Use aim override. */
		if (AimOverride.IsValid())
		{
			FRotator RawRotation = GetOwningCamera()->GetCameraComponent()->GetRelativeRotation();
			FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetOwningCamera()->GetCameraComponent()->GetComponentLocation(), AimOverride.Get()->GetActorLocation());
			GetOwningCamera()->GetCameraComponent()->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, RawRotation.Roll));
		}
		
		ElapsedTime += DeltaTime;
		ElapsedFrames = FFrameTime::FromDecimal(ElapsedTime * SequencePlayer->GetFrameRate().AsDecimal() * SequencePlayer->GetPlayRate());
		if (ElapsedFrames.FrameNumber >= SequencePlayer->GetFrameDuration())
		{
			OwningSettingComponent->GetECameraManager()->TerminateActiveCamera();
		}
	}
}

void UKeyframeExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	ElapsedFrames.FrameNumber = 0;
	ElapsedTime = 0.f;

	Initialize();
}

void UKeyframeExtension::Initialize()
{
	if (ActorSequenceComponent == nullptr)
	{
		ActorSequenceComponent = GetOwningActor()->FindComponentByClass<UActorSequenceComponent>();
	}

	if (ActorSequenceComponent != nullptr)
	{
		MovieSequence = ActorSequenceComponent->GetSequence();
		SequencePlayer = ActorSequenceComponent->GetSequencePlayer();
	}
}