// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ECameraHUD.generated.h"

class UControlAim;

/**
 *  This HUD class is used to show camera debug info on screen.
 */
UCLASS()
class COMPONENTCAMERASYSTEM_API AECameraHUD : public AHUD
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "ECameraHUD")
	bool bShowCameraDebug = true;

	const FLinearColor FollowPositionColor = FLinearColor(0.95f, 0.55f, 0.55f, 0.8f);
	const FLinearColor AimPositionColor = FLinearColor(0.6f, 0.78f, 1.0f, 0.8f);
	const FLinearColor RectColor = FLinearColor(1.f, 1.f, 1.f, 0.4f);
	const FLinearColor MiscColor = FLinearColor(0.8f, 0.9f, 0.8f, 0.8f);

public:
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	void DrawRectOnScreenWithOffset(FLinearColor Color, FVector2f& ScreenOffset, FVector2f& ScreenOffsetWidth, FVector2f& ScreenOffsetHeight);
	void DrawRectOnScreenWithPosition(FLinearColor Color, FVector& Position);
	void DrawRectOnScreenForGroupActors(FLinearColor Color, TArray<FBoundingWrappedActor>& TargetActors);
	void DrawRectOnScreenForAimAssist(FLinearColor Color, UControlAim* ControlAim, const FAimAssist& AimAssist);
};
