// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/AnimatedCameraExtension.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimationPoseData.h"
#include "Curves/SimpleCurve.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Core/ECameraManager.h"
#include "Core/ECameraSettingsComponent.h"


UAnimatedCameraExtension::UAnimatedCameraExtension()
{

}

void UAnimatedCameraExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (AnimToPlay)
	{
		/** Evaluate bone transform at current time, and apply offset. */
		FTransform OutTransform;
		AnimToPlay->GetBoneTransform(OutTransform, 0, ElapsedTime, true);
		OutTransform.SetLocation(OutTransform.GetLocation() + PositionOffset);

		/** Get ref frame. */
		FTransform RefFrame = RefCoordinateActor == nullptr ? RefCoordinate : RefCoordinateActor->GetActorTransform();

		/** Get transform in world space relative to the ref frame. */
		FTransform NewTransform;
		NewTransform.SetLocation(UKismetMathLibrary::TransformLocation(RefFrame, OutTransform.GetLocation()));
		NewTransform.SetRotation(UKismetMathLibrary::TransformRotation(RefFrame, OutTransform.GetRotation().Rotator()).Quaternion());

		/** Set transform. */
		GetOwningActor()->SetActorTransform(NewTransform);

 
		/** Set FOV. */
#if WITH_EDITOR
		if (SimpleCurve.HasAnyData())
		{
			float FOV = SimpleCurve.Eval(ElapsedTime, 90.0f);
			GetOwningCamera()->GetCameraComponent()->FieldOfView = FOV;
		}
#else
		EvaluateFOV();
#endif

		ElapsedTime += DeltaTime;
		if (ElapsedTime >= AnimToPlay->GetPlayLength())
		{
			/** Terminate this camera. */
			OwningSettingComponent->GetECameraManager()->TerminateActiveCamera();
		}
	}
}

void UAnimatedCameraExtension::EvaluateFOV()
{
	// Prepare the requisite data
	USkeleton* Skeleton = AnimToPlay->GetSkeleton();
	const int32 BoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(FName("root"));

	TArray<FBoneIndexType> RequiredBones;
	RequiredBones.Add(BoneIndex);
	FBoneContainer BoneContainer(RequiredBones, false, *Skeleton);

	FCompactPose Pose;
	Pose.SetBoneContainer(&BoneContainer);

	FBlendedCurve Curve;
	Curve.InitFrom(BoneContainer);

	FStackCustomAttributes InAttributes;

	// Evaluate attribute curves
	FAnimationPoseData AnimationPoseData(Pose, Curve, InAttributes);
	FAnimExtractContext Context(ElapsedTime, false);
	AnimToPlay->GetCustomAttributes(AnimationPoseData, Context, false);

	// Access the FOV attribute
	float FOV;

	FStackCustomAttributes& OutAttributes = AnimationPoseData.GetAttributes();
	if (OutAttributes.GetBoneAttribute<float>(FCompactPoseBoneIndex(0), FName("FOV"), FOV))
	{
		GetOwningCamera()->GetCameraComponent()->FieldOfView = FOV;
	}
}

void UAnimatedCameraExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	ElapsedTime = 0.f;

	/** Cache FOV curve at the very beginning to save performance. */
#if WITH_EDITOR
	if (AnimToPlay)
	{
		TArray<FCustomAttribute> Attributes;
		AnimToPlay->GetCustomAttributesForBone(FName("root"), Attributes);

		if (Attributes.Num() > 0)
		{
			for (const FCustomAttribute& Attribute : Attributes)
			{
				if (Attribute.Name.ToString().Contains("FOV"))
				{
					SimpleCurve = FSimpleCurve();
					TArray<FSimpleCurveKey> SimpleCurveKeys;

					for (int i = 0; i < Attribute.Times.Num(); ++i)
					{
						SimpleCurveKeys.Add(FSimpleCurveKey(Attribute.Times[i], Attribute.Values[i].GetValue<float>()));
					}

					SimpleCurve.SetKeys(SimpleCurveKeys);
					break;
				}
			}
		}
	}
#endif

	//I don't know if this line is needed. Need experiments.
	//GetOwningCamera()->GetCameraComponent()->FieldOfView = GetOwningCamera()->DefaultFOV;
}