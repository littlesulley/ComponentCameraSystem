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
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/ECameraLibrary.h"
#include "Core/ECameraSubsystem.h"

UKeyframeExtension::UKeyframeExtension()
{
	PCMGParams = FPCMGParams();
	Coordinate = FTransform();
	bCoordinateLocationOnly = false;
	LocationOffset = FVector();
}

void UKeyframeExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (SequencePlayer)
	{
		/** Use the internal SetPlaybackPosition function to immediately set play back position.  */
		SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(ElapsedFrames, EUpdatePositionMethod::Play));

		/** Get coordinate frame and apply keyframes. */
		FTransform Frame = GetCoordinateTransform();

		GetOwningCamera()->GetCameraComponent()->SetWorldLocation(UKismetMathLibrary::TransformLocation(Frame, GetOwningCamera()->GetCameraComponent()->GetRelativeLocation() + LocationOffset));
		
		if (!bCoordinateLocationOnly)
		{
			GetOwningCamera()->GetCameraComponent()->SetWorldRotation(UKismetMathLibrary::TransformRotation(Frame, GetOwningCamera()->GetCameraComponent()->GetRelativeRotation()));
		}
		else
		{
			GetOwningCamera()->GetCameraComponent()->SetWorldRotation(GetOwningCamera()->GetCameraComponent()->GetRelativeRotation());
		}

		/** Aim override. */
		if (IsValid(AimOverride))
		{
			FRotator RawRotation = GetOwningCamera()->GetCameraComponent()->GetRelativeRotation();
			FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetOwningCamera()->GetCameraComponent()->GetComponentLocation(), GetAimDestLocation());
			GetOwningCamera()->GetCameraComponent()->SetWorldRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, RawRotation.Roll));
		}
		
		ElapsedTime += DeltaTime;
		ElapsedFrames = FFrameTime::FromDecimal(ElapsedTime * SequencePlayer->GetFrameRate().AsDecimal() * SequencePlayer->GetPlayRate());
		if (ElapsedFrames.FrameNumber >= SequencePlayer->GetFrameDuration())
		{
			UECameraLibrary::TerminateActiveCamera(this);
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

FTransform UKeyframeExtension::GetCoordinateTransform()
{
	if (!IsValid(CoordinateActor))
	{
		return Coordinate;
	}

	FTransform Frame = CoordinateActor->GetActorTransform();

	if (CoordinateSocket.IsNone() || !CoordinateSocket.IsValid())
	{
		return Frame;
	}

	UActorComponent* ActorComponent = CoordinateActor->GetComponentByClass(USkeletalMeshComponent::StaticClass());

	if (ActorComponent == nullptr)
	{
		return Frame;
	}

	USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

	if (SkeletonComponent->DoesSocketExist(CoordinateSocket))
	{
		return SkeletonComponent->GetSocketTransform(CoordinateSocket);
	}

	return Frame;
}

FVector UKeyframeExtension::GetAimDestLocation()
{
	FVector DestLocation = AimOverride.Get()->GetActorLocation();

	if (AimSocket.IsNone() || !AimSocket.IsValid())
	{
		return DestLocation;
	}

	UActorComponent* ActorComponent = AimOverride->GetComponentByClass(USkeletalMeshComponent::StaticClass());

	if (ActorComponent == nullptr)
	{
		return DestLocation;
	}

	USkeletalMeshComponent* SkeletonComponent = Cast<USkeletalMeshComponent>(ActorComponent);

	if (SkeletonComponent->DoesSocketExist(AimSocket))
	{
		return SkeletonComponent->GetSocketLocation(AimSocket);
	}

	return DestLocation;
}