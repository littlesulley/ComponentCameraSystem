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

void UKeyframeExtension::TossSequence()
{
	if (MovieSequence == nullptr)
	{
		Initialize();
	}

	if (MovieSequence != nullptr)
	{
		TArrayView<FMovieSceneDoubleChannel*> Channels = GetTransformChannels(MovieSequence);

		if (Channels.Num() != 0)
		{
			if (!bModified)
			{
				DuplicateRawData(Channels);
				bModified = true;
			}

			for (FMovieSceneDoubleChannel* Channel : Channels)
			{
				TossChannel(Channel);
			}
		}
	}
}

void UKeyframeExtension::TossSequence(UActorSequenceComponent* InActorSequenceComponent)
{
	/*if (InActorSequenceComponent)
	{
		TArray<float> Input;
		TArray<float> Output;
		UPCMGNeuralNetwork* Network = nullptr;

		UActorSequence* InMovieSequence = InActorSequenceComponent->GetSequence();

		if (InMovieSequence)
		{
			TArrayView<FMovieSceneDoubleChannel*> Channels = GetTransformChannels(InMovieSequence);

			if (Channels.Num() < 9)
			{
				return;
			}
			
			switch (PCMGParams.Model)
			{
				case EPCMGModel::PPO:
				case EPCMGModel::SAC:
				{
					Network = NewObject<UPCMGNeuralNetwork>((UObject*)GetTransientPackage(), UPCMGNeuralNetwork::StaticClass());

					if (Network != nullptr)
					{
						if (!bModified)
						{
							DuplicateRawData(Channels);
							bModified = true;
						}

						ConstructInput(Input, Channels);
						Network->RunModel(PCMGParams.Model, Input, Output);
						ApplyOutput(Output, Channels, InMovieSequence);
					}
				}
				break;
				case EPCMGModel::Magnetic:
				{

				}
				break;
				case EPCMGModel::Function:
				{
					GetFunctionOutput(PCMGParams.FunctionParams, Output);
					ApplyOutput(Output, Channels, InMovieSequence);
				}
				break;
				default: { }
			}
		}
	}*/
}

void UKeyframeExtension::GetFunctionOutput(FFunctionParamsCollection FunctionParams, TArray<float>& Output)
{
	for (int i = 0; i <= 25; ++i)
	{
		float CurrentTime = i * 0.2f;
		float X = FunctionParams.XFuncParams.A1 * FMath::Exp(FunctionParams.XFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.XFuncParams.B1 * CurrentTime + FunctionParams.XFuncParams.B2)
				+ FunctionParams.XFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.XFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.XFuncParams.C3 * CurrentTime
				+ FunctionParams.XFuncParams.D;
		float Y = FunctionParams.YFuncParams.A1 * FMath::Exp(FunctionParams.YFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.YFuncParams.B1 * CurrentTime + FunctionParams.YFuncParams.B2)
				+ FunctionParams.YFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.YFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.YFuncParams.C3 * CurrentTime
				+ FunctionParams.YFuncParams.D;
		float Z = FunctionParams.ZFuncParams.A1 * FMath::Exp(FunctionParams.ZFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.ZFuncParams.B1 * CurrentTime + FunctionParams.ZFuncParams.B2)
				+ FunctionParams.ZFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.ZFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.ZFuncParams.C3 * CurrentTime
				+ FunctionParams.ZFuncParams.D;
		float Roll = FunctionParams.RollFuncParams.A1 * FMath::Exp(FunctionParams.RollFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.RollFuncParams.B1 * CurrentTime + FunctionParams.RollFuncParams.B2)
				+ FunctionParams.RollFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.RollFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.RollFuncParams.C3 * CurrentTime
				+ FunctionParams.RollFuncParams.D;
		float Pitch = FunctionParams.PitchFuncParams.A1 * FMath::Exp(FunctionParams.PitchFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.PitchFuncParams.B1 * CurrentTime + FunctionParams.PitchFuncParams.B2)
				+ FunctionParams.PitchFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.PitchFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.PitchFuncParams.C3 * CurrentTime
				+ FunctionParams.PitchFuncParams.D;
		float Yaw = FunctionParams.YawFuncParams.A1 * FMath::Exp(FunctionParams.YawFuncParams.A2 * CurrentTime)
				* FMath::Sin(FunctionParams.YawFuncParams.B1 * CurrentTime + FunctionParams.YawFuncParams.B2)
				+ FunctionParams.YawFuncParams.C1 * CurrentTime * CurrentTime * CurrentTime
				+ FunctionParams.YawFuncParams.C2 * CurrentTime * CurrentTime
				+ FunctionParams.YawFuncParams.C3 * CurrentTime
				+ FunctionParams.YawFuncParams.D;

		Output.Add(X);
		Output.Add(Y);
		Output.Add(Z);
		Output.Add(Roll);
		Output.Add(Pitch);
		Output.Add(Yaw);
	}
}

void UKeyframeExtension::ConstructInput(TArray<float>& Input, const TArrayView<FMovieSceneDoubleChannel*>& Channels)
{
	//Input.Add(PCMGParams.PosTurbulence);
	//Input.Add(PCMGParams.RotTurbulence);

	//Input.Add(PCMGParams.PosSymmetry);
	//Input.Add(PCMGParams.RotSymmetry);

	//Input.Add(PCMGParams.PosIncreaseMonotonicity[0]);
	//Input.Add(PCMGParams.PosIncreaseMonotonicity[1]);
	//Input.Add(PCMGParams.PosIncreaseMonotonicity[2]);
	//Input.Add(PCMGParams.PosDecreaseMonotonicity[0]);
	//Input.Add(PCMGParams.PosDecreaseMonotonicity[1]);
	//Input.Add(PCMGParams.PosDecreaseMonotonicity[2]);
	//Input.Add(PCMGParams.RotIncreaseMonotonicity[0]);
	//Input.Add(PCMGParams.RotIncreaseMonotonicity[1]);
	//Input.Add(PCMGParams.RotIncreaseMonotonicity[2]);
	//Input.Add(PCMGParams.RotDecreaseMonotonicity[0]);
	//Input.Add(PCMGParams.RotDecreaseMonotonicity[1]);
	//Input.Add(PCMGParams.RotDecreaseMonotonicity[2]);

	Input.Add(PCMGParams.Ranges.XVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.XVelocityRange[1]);
	Input.Add(PCMGParams.Ranges.YVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.YVelocityRange[1]);
	Input.Add(PCMGParams.Ranges.ZVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.ZVelocityRange[1]);

	Input.Add(PCMGParams.Ranges.RollVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.RollVelocityRange[1]);
	Input.Add(PCMGParams.Ranges.PitchVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.PitchVelocityRange[1]);
	Input.Add(PCMGParams.Ranges.YawVelocityRange[0]);
	Input.Add(PCMGParams.Ranges.YawVelocityRange[1]);
	/*
	float XAccelerationMinimum     = FMath::Max<float>(PCMGParams.Ranges.XAccelerationRange[0],     PCMGParams.Ranges.XVelocityRange[0] - PCMGParams.Ranges.XVelocityRange[1]);
	float XAccelerationMaximum     = FMath::Min<float>(PCMGParams.Ranges.XAccelerationRange[1],	    PCMGParams.Ranges.XVelocityRange[1] - PCMGParams.Ranges.XVelocityRange[0]);
	float YAccelerationMinimum     = FMath::Max<float>(PCMGParams.Ranges.YAccelerationRange[0],		PCMGParams.Ranges.YVelocityRange[0] - PCMGParams.Ranges.YVelocityRange[1]);
	float YAccelerationMaximum     = FMath::Min<float>(PCMGParams.Ranges.YAccelerationRange[1],		PCMGParams.Ranges.YVelocityRange[1] - PCMGParams.Ranges.YVelocityRange[0]);
	float ZAccelerationMinimum     = FMath::Max<float>(PCMGParams.Ranges.ZAccelerationRange[0],		PCMGParams.Ranges.ZVelocityRange[0] - PCMGParams.Ranges.ZVelocityRange[1]);
	float ZAccelerationMaximum     = FMath::Min<float>(PCMGParams.Ranges.ZAccelerationRange[1],		PCMGParams.Ranges.ZVelocityRange[1] - PCMGParams.Ranges.ZVelocityRange[0]);
	float RollAccelerationMinimum  = FMath::Max<float>(PCMGParams.Ranges.RollAccelerationRange[0],  PCMGParams.Ranges.RollVelocityRange[0] - PCMGParams.Ranges.RollVelocityRange[1]);
	float RollAccelerationMaximum  = FMath::Min<float>(PCMGParams.Ranges.RollAccelerationRange[1],  PCMGParams.Ranges.RollVelocityRange[1] - PCMGParams.Ranges.RollVelocityRange[0]);
	float PitchAccelerationMinimum = FMath::Max<float>(PCMGParams.Ranges.PitchAccelerationRange[0], PCMGParams.Ranges.PitchVelocityRange[0] - PCMGParams.Ranges.PitchVelocityRange[1]);
	float PitchAccelerationMaximum = FMath::Min<float>(PCMGParams.Ranges.PitchAccelerationRange[1], PCMGParams.Ranges.PitchVelocityRange[1] - PCMGParams.Ranges.PitchVelocityRange[0]);
	float YawAccelerationMinimum   = FMath::Max<float>(PCMGParams.Ranges.YawAccelerationRange[0],   PCMGParams.Ranges.YawVelocityRange[0] - PCMGParams.Ranges.YawVelocityRange[1]);
	float YawAccelerationMaximum   = FMath::Min<float>(PCMGParams.Ranges.YawAccelerationRange[1],   PCMGParams.Ranges.YawVelocityRange[1] - PCMGParams.Ranges.YawVelocityRange[0]);
	*/
	Input.Add(PCMGParams.Ranges.XAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.XAccelerationRange[1]);
	Input.Add(PCMGParams.Ranges.YAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.YAccelerationRange[1]);
	Input.Add(PCMGParams.Ranges.ZAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.ZAccelerationRange[1]);
	Input.Add(PCMGParams.Ranges.RollAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.RollAccelerationRange[1]);
	Input.Add(PCMGParams.Ranges.PitchAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.PitchAccelerationRange[1]);
	Input.Add(PCMGParams.Ranges.YawAccelerationRange[0]);
	Input.Add(PCMGParams.Ranges.YawAccelerationRange[1]);

	for (int i = 0; i < 54; ++i)
		Input.Add(0);

	for (int i = 0; i < 6; ++i)
	{
		FMovieSceneDoubleChannel* Channel = Channels[i];
		TArrayView<FMovieSceneDoubleValue> Values = Channel->GetData().GetValues();
		if (Values.Num() > 0)
		{
			Input.Add(Values[0].Value);
		}
		else
		{
			Input.Add(0);
		}
	}

	check(Input.Num() == 84);
}

void UKeyframeExtension::ApplyOutput(TArray<float>& Output, TArrayView<FMovieSceneDoubleChannel*>& Channels, UActorSequence* InMovieSequence)
{
	check(Output.Num() == 156);
	
	UMovieScene* MovieScene = InMovieSequence->GetMovieScene();

	/* See https://forums.unrealengine.com/t/changing-sequencer-section-end-range-in-c/472819/4. */
	int TickResolution = MovieScene->GetTickResolution().AsDecimal();
	int DisplayRate = MovieScene->GetDisplayRate().AsDecimal();
	int Ratio = TickResolution / DisplayRate;
	
	TArray<FFrameNumber> NewTransformTimes[6];
	TArray<FMovieSceneDoubleValue> NewTransformValues[6];

	const int NumberOfSeconds = 5;
	const int FramesPerSecond = 5;

	for (int Second = 0; Second <= NumberOfSeconds; ++Second)
		for (int Frame = 0; Frame < FramesPerSecond; ++Frame)
		{
			if (Second == NumberOfSeconds && Frame != 0)
				break;

			int Index = (FramesPerSecond * Second + Frame) * 6;
			float X            = Output[Index];
			float Y            = Output[Index + 1];
			float Z            = Output[Index + 2];
			float Roll	       = Output[Index + 3];
			float Pitch        = Output[Index + 4];
			float Yaw          = Output[Index + 5];

			FMovieSceneDoubleValue XValue(X);
			FMovieSceneDoubleValue YValue(Y);
			FMovieSceneDoubleValue ZValue(Z);
			FMovieSceneDoubleValue RollValue (Roll);
			FMovieSceneDoubleValue PitchValue(Pitch);
			FMovieSceneDoubleValue YawValue  (Yaw);

			XValue.InterpMode     = RCIM_Cubic;
			YValue.InterpMode	  = RCIM_Cubic;
			ZValue.InterpMode	  = RCIM_Cubic;
			RollValue.InterpMode  = RCIM_Cubic;
			PitchValue.InterpMode = RCIM_Cubic;
			YawValue.InterpMode   = RCIM_Cubic;

			XValue.TangentMode    = RCTM_Auto;
			YValue.TangentMode    = RCTM_Auto;
			ZValue.TangentMode    = RCTM_Auto;
			RollValue.TangentMode = RCTM_Auto;
			XValue.TangentMode    = RCTM_Auto;
			YawValue.TangentMode  = RCTM_Auto;

			NewTransformValues[0].Add(XValue);
			NewTransformValues[1].Add(YValue);
			NewTransformValues[2].Add(ZValue);
			NewTransformValues[3].Add(RollValue);
			NewTransformValues[4].Add(PitchValue);
			NewTransformValues[5].Add(YawValue);

			int32 CurrentFrame = Ratio * (30 * Second + 30 / FramesPerSecond * Frame);

			NewTransformTimes[0].Add(FFrameNumber(CurrentFrame));
			NewTransformTimes[1].Add(FFrameNumber(CurrentFrame));
			NewTransformTimes[2].Add(FFrameNumber(CurrentFrame));
			NewTransformTimes[3].Add(FFrameNumber(CurrentFrame));
			NewTransformTimes[4].Add(FFrameNumber(CurrentFrame));
			NewTransformTimes[5].Add(FFrameNumber(CurrentFrame));
		}

	for (int i = 0; i < 6; ++i)
	{
		Channels[i]->Set(NewTransformTimes[i], NewTransformValues[i]);
		Channels[i]->AutoSetTangents();
	}
}

void UKeyframeExtension::TossChannel(FMovieSceneDoubleChannel* Channel)
{
	TMovieSceneChannelData<FMovieSceneDoubleValue> RawData = Channel->GetData();
	TArrayView<FMovieSceneDoubleValue> Values = RawData.GetValues();
	for (FMovieSceneDoubleValue& Value : Values)
	{
		Value.Value = 10.0f;
	}
}

void UKeyframeExtension::Recover()
{
	if (MovieSequence == nullptr)
	{
		Initialize();
	}

	if (MovieSequence != nullptr && bModified)
	{
		TArrayView<FMovieSceneDoubleChannel*> Channels = GetTransformChannels(MovieSequence);

		if (Channels.Num() != 0)
		{
			bModified = false;

			for (int index = 0; index < Channels.Num(); ++index)
			{
				Channels[index]->Set(RawTransformTimes[index], RawTransformValues[index]);
			}
		}
	}
}

void UKeyframeExtension::Recover(UActorSequenceComponent* InActorSequenceComponent)
{
	if (InActorSequenceComponent && bModified)
	{
		UActorSequence* InMovieSequence = InActorSequenceComponent->GetSequence();

		if (InMovieSequence)
		{
			TArrayView<FMovieSceneDoubleChannel*> Channels = GetTransformChannels(InMovieSequence);

			if (Channels.Num() != 0)
			{
				bModified = false;

				for (int index = 0; index < Channels.Num(); ++index)
				{
					Channels[index]->Set(RawTransformTimes[index], RawTransformValues[index]);
				}
			}
		}
	}
}

void UKeyframeExtension::Save()
{
	bModified = false;
}

void UKeyframeExtension::Save(UActorSequenceComponent* InActorSequenceComponent)
{
	bModified = false;
}

TArrayView<FMovieSceneDoubleChannel*> UKeyframeExtension::GetTransformChannels(UActorSequence* InMovieSceneSequence)
{
	if (InMovieSceneSequence)
	{
		UMovieScene* MovieScene = InMovieSceneSequence->GetMovieScene();

		/** Get all bindings from movie scene. */
		const TArray<FMovieSceneBinding>& Bindings = MovieScene->GetBindings();

		/** Only has two bindings: the actor itself and the camera component. */
		if (Bindings.Num() != 2) return TArrayView<FMovieSceneDoubleChannel*>();

		/** Camera component is the first binding. */
		const FMovieSceneBinding& CameraBinding = Bindings[0];
		const TArray<UMovieSceneTrack*>& Tracks = CameraBinding.GetTracks();

		for (UMovieSceneTrack* Track : Tracks)
		{
			/** We only manipulate the transform track. */
			if (Track->GetName().Contains("Transform"))
			{
				const TArray<UMovieSceneSection*>& Sections = Track->GetAllSections();
				if (Sections.Num() > 0)
				{
					TArrayView<FMovieSceneDoubleChannel*> Channels = Sections[0]->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();
					return Channels;
				}
			}
		}
	}
	
	return TArrayView<FMovieSceneDoubleChannel*>();
}

void UKeyframeExtension::DuplicateRawData(TArrayView<FMovieSceneDoubleChannel*> Channels)
{
	for (int index = 0; index < Channels.Num(); ++index)
	{
		RawTransformTimes[index] = Channels[index]->GetTimes();
		RawTransformValues[index] = Channels[index]->GetValues();
	}
}