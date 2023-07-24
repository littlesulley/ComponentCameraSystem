// Copyright 2023 by Sulley. All Rights Reserved.

#include "Cameras/EArchVizCineCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

using UMath = UKismetMathLibrary;

UEArchVizCineCameraComponent::UEArchVizCineCameraComponent(const FObjectInitializer& ObjectInitializer)
{
	bEnableCorrection = true;
	PerspectiveStrength = 1.0f;
	MinMaxPitch = FVector2D(-70.0, 70.0);
}

void UEArchVizCineCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);

	if (bEnableCorrection)
	{
		float CurrentPitch = FMath::Clamp(DesiredView.Rotation.Pitch, MinMaxPitch[0], MinMaxPitch[1]);
		float DesiredPitch = CurrentPitch * (1.0f - PerspectiveStrength);
		float VerticalFOV = 2 * UMath::DegAtan(UMath::DegTan(DesiredView.FOV / 2.0f) / DesiredView.AspectRatio);

		VerticalProjectOffset = UMath::DegTan(CurrentPitch - DesiredPitch) / UMath::DegTan(VerticalFOV / 2.0f);

		DesiredView.Rotation.Pitch = DesiredPitch;
		DesiredView.OffCenterProjectionOffset = FVector2D(0, VerticalProjectOffset);
	}
	else
	{
		DesiredView.OffCenterProjectionOffset = FVector2D(0, 0);
	}
}

#if WITH_EDITOR
void UEArchVizCineCameraComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetName());

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UEArchVizCameraComponent, PerspectiveStrength))
		{
			// @TODO: Placeholder for future implementation, if needed
		}
	}
}
#endif
