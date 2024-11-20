// Copyright 2023 by Sulley. All Rights Reserved.

#include "Extensions/AnimatedCameraExtension.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AttributeCurve.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Animation/BuiltInAttributeTypes.h"
#include "Animation/AttributesContainer.h"
#include "Animation/AttributesRuntime.h"
#include "Animation/AnimationPoseData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Core/ECameraSubsystem.h"
#include "Core/ECameraSettingsComponent.h"
#include "Utils/ECameraLibrary.h"


UAnimatedCameraExtension::UAnimatedCameraExtension()
{

}

void UAnimatedCameraExtension::UpdateComponent_Implementation(float DeltaTime)
{
	if (AnimToPlay)
	{
		/** Evaluate bone transform at current time, and apply offset. */
		FTransform OutTransform;
		AnimToPlay->GetBoneTransform(OutTransform, FSkeletonPoseBoneIndex(0), ElapsedTime, true);
		OutTransform.SetLocation(OutTransform.GetLocation() + PositionOffset);

		/** Get ref frame. */
		FTransform RefFrame = CoordinateActor == nullptr ? Coordinate : CoordinateActor->GetActorTransform();

		/** Get transform in world space relative to the ref frame. */
		FTransform NewTransform;
		NewTransform.SetLocation(UKismetMathLibrary::TransformLocation(RefFrame, OutTransform.GetLocation()));
		NewTransform.SetRotation(UKismetMathLibrary::TransformRotation(RefFrame, OutTransform.GetRotation().Rotator()).Quaternion());

		/** Set transform. */
		GetOwningActor()->SetActorTransform(NewTransform);

		/** Set FOV. */
#if WITH_EDITOR
		TArrayView<const FAnimatedBoneAttribute> Attributes = AnimToPlay->GetDataModel()->GetAttributes();
		if (Attributes.Num() > 0)
		{
			for (const FAnimatedBoneAttribute& Attribute : Attributes)
			{
				if (Attribute.Identifier.GetName().ToString().Contains("FOV"))
				{
					float FOV = Attribute.Curve.Evaluate<FFloatAnimationAttribute>(ElapsedTime).Value;
					GetOwningCamera()->GetCameraComponent()->FieldOfView = FOV;
					break;
				}
			}
		}
#else
		EvaluateFOV();
#endif

		ElapsedTime += DeltaTime;
		if (ElapsedTime >= AnimToPlay->GetPlayLength())
		{
			/** Terminate this camera. */
			UECameraLibrary::TerminateActiveCamera(this);
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

	UE::Anim::FStackAttributeContainer InAttributes;

	// Evaluate attribute curves
	FAnimationPoseData AnimationPoseData(Pose, Curve, InAttributes);
	FAnimExtractContext Context((double)ElapsedTime, false);
	AnimToPlay->EvaluateAttributes(AnimationPoseData, Context, false);

	// Access the FOV attribute
	UE::Anim::FStackAttributeContainer& OutAttributes = AnimationPoseData.GetAttributes();
	int TypeIndex = OutAttributes.FindTypeIndex(FFloatAnimationAttribute::StaticStruct());
	const TArray<UE::Anim::FAttributeId>& AttributeIds = OutAttributes.GetKeys(TypeIndex);
	for (UE::Anim::FAttributeId AttributeId : AttributeIds)
	{
		if (AttributeId.GetName().ToString().Contains("FOV"))
		{
			FFloatAnimationAttribute* FOVAttribute = OutAttributes.Find<FFloatAnimationAttribute>(AttributeId);
			float FOV = FOVAttribute->Value;
			GetOwningCamera()->GetCameraComponent()->FieldOfView = FOV;
			break;
		}
	}
}

void UAnimatedCameraExtension::ResetOnBecomeViewTarget(APlayerController* PC, bool bPreserveState)
{
	ElapsedTime = 0.f;
	//I don't know if this line is needed. Need experiments.
	//GetOwningCamera()->GetCameraComponent()->FieldOfView = GetOwningCamera()->DefaultFOV;
}