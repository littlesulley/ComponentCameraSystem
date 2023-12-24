// Copyright 2023 by Sulley. All Rights Reserved.

#include "Utils/ECameraHUD.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Canvas.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "CameraRig_Crane.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraManager.h"
#include "Components/ECameraComponentFollow.h"
#include "Components/ECameraComponentAim.h"
#include "Components/FramingFollow.h"
#include "Components/SimpleFollow.h"
#include "Components/RailFollow.h"
#include "Components/OrbitFollow.h"
#include "Components/CraneFollow.h"
#include "Components/HardLockFollow.h"
#include "Components/TargetingAim.h"
#include "Components/HardLockAim.h"
#include "Components/ControlAim.h"
#include "Extensions/ConfinerExtension.h"
#include "Extensions/DeoccluderExtension.h"
#include "Extensions/ECameraExtensionBase.h"
#include "Extensions/ResolveGroupActorExtension.h"
#include "Utils/ECameraGroupActor.h"
#include "Utils/ECameraGroupActorComponent.h"

void AECameraHUD::BeginPlay()
{
	Super::BeginPlay();

	if (ECameraManager == nullptr)
	{
		UWorld* World = GetWorld();
		AECameraManager* Manager = Cast<AECameraManager>(UGameplayStatics::GetActorOfClass(World, AECameraManager::StaticClass()));

		if (Manager == nullptr)
		{
			Manager = World->SpawnActor<AECameraManager>();
		}
		
		ECameraManager = Manager;
	}
}

void AECameraHUD::DrawHUD()
{
	Super::DrawHUD();

#if ENABLE_DRAW_DEBUG
	if (bShowCameraDebug && ECameraManager)
	{
		AECameraBase* ActiveCamera = ECameraManager->GetActiveCamera();
		if (ActiveCamera && ActiveCamera->GetSettingsComponent()->IsActive())
		{
			UECameraSettingsComponent* CameraSettingsComponent = ActiveCamera->GetSettingsComponent();

			UECameraComponentFollow* FollowComponent = CameraSettingsComponent->GetFollowComponent();
			if (FollowComponent)
			{
				/** For FramingFollow. */
				if (FollowComponent->IsA<UFramingFollow>())
				{
					UFramingFollow* FramingFollow = Cast<UFramingFollow>(FollowComponent);
					FVector2D ScreenOffset = FramingFollow->GetScreenOffset();
					FVector2D ScreenOffsetWidth = FramingFollow->GetScreenOffsetWidth();
					FVector2D ScreenOffsetHeight = FramingFollow->GetScreenOffsetHeight();
					
					DrawRectOnScreenWithOffset(RectColor, ScreenOffset, ScreenOffsetWidth, ScreenOffsetHeight);

					FVector WorldLocation = FramingFollow->GetFollowPosition();
					DrawRectOnScreenWithPosition(FollowPositionColor, WorldLocation);
				}
				/** For SimpleFollow. */
				else if (FollowComponent->IsA<USimpleFollow>())
				{
					// Do nothing for this class.
				}
				/** For RailFollow. */
				else if (FollowComponent->IsA<URailFollow>())
				{
					// Do nothing for this class. Has already been resolved internally.
				}
				/** For CraneFollow. */
				else if (FollowComponent->IsA<UCraneFollow>())
				{
					// Do nothing for this class.
				}
				/** For OrbitFollow. */
				else if (FollowComponent->IsA<UOrbitFollow>())
				{
					UOrbitFollow* OrbitFollow = Cast<UOrbitFollow>(FollowComponent);
					for (FOrbit Orbit : OrbitFollow->GetOrbits())
					{
						FVector Center = OrbitFollow->GetFollowTarget()->GetActorLocation() + FVector(0, 0, Orbit.Height);
						DrawDebugCircle(GetWorld(), Center, Orbit.Radius, 50, MiscColor.ToFColor(false), false, 0.0f, 0U, 1.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);
					}
				}
				/** For HardLockFollow. */
				else if (FollowComponent->IsA<UHardLockFollow>())
				{
					// Do nothing for this class.
				}
			}

			UECameraComponentAim* AimComponent = CameraSettingsComponent->GetAimComponent();
			if (AimComponent)
			{
				/** For TargetingAim. */
				if (AimComponent->IsA<UTargetingAim>())
				{
					UTargetingAim* TargetingAim = Cast<UTargetingAim>(AimComponent);
					FVector2D ScreenOffset = TargetingAim->GetScreenOffset();
					FVector2D ScreenOffsetWidth = TargetingAim->GetScreenOffsetWidth();
					FVector2D ScreenOffsetHeight = TargetingAim->GetScreenOffsetHeight();

					DrawRectOnScreenWithOffset(RectColor, ScreenOffset, ScreenOffsetWidth, ScreenOffsetHeight);

					FVector WorldLocation = TargetingAim->GetRealAimPosition(true);
					DrawRectOnScreenWithPosition(AimPositionColor, WorldLocation);
				}
				/** For ControlAim. */
				else if (AimComponent->IsA<UControlAim>())
				{
					UControlAim* ControlAim = Cast<UControlAim>(AimComponent);
					DrawRect(RectColor, Canvas->SizeX / 2 - 5.f, Canvas->SizeY / 2 - 5.f, 10.f, 10.f);
					
					const FAimAssist& AimAssist = ControlAim->GetAimAssist();
					DrawRectOnScreenForAimAssist(RectColor, ControlAim, AimAssist);
					
				}
				/** For HardLockAim. */
				else if (AimComponent->IsA<UHardLockAim>())
				{
					// Do nothing for this class.
				}
			}

			for (UECameraExtensionBase* Extension : CameraSettingsComponent->GetExtensions())
			{
				if (Extension)
				{
					/** For DeoccluderExtension. */
					if (Extension->IsA<UDeoccluderExtension>())
					{
						// Do nothing for this class. Has already been resolved internally.
					}
					/** For ConfinerExtension. */
					else if (Extension->IsA<UConfinerExtension>())
					{
						UConfinerExtension* Confiner = Cast<UConfinerExtension>(Extension);
						TArray<TSoftObjectPtr<ATriggerBox>>& Boxes = Confiner->GetBoxes();
						for (TSoftObjectPtr<ATriggerBox> BoxPtr : Boxes)
						{
							ATriggerBox* Box = BoxPtr.Get();
							UBoxComponent* BoxCollisionComponent = CastChecked<UBoxComponent>(Box->GetCollisionComponent());
							DrawDebugBox(GetWorld(), Box->GetActorLocation(), BoxCollisionComponent->GetScaledBoxExtent(), Box->GetActorRotation().Quaternion(), MiscColor.ToFColor(false), false, 0.0f, 0U, 1.0f);
						}

					}
					/** For ResolveGroupActorExtension. */
					else if (Extension->IsA<UResolveGroupActorExtension>())
					{
						UResolveGroupActorExtension* GroupActorResolver = Cast<UResolveGroupActorExtension>(Extension);
						AECameraGroupActor* FollowGroupActor = Cast<AECameraGroupActor>(GroupActorResolver->GetOwningSettingComponent()->GetFollowTarget());
						AECameraGroupActor* AimGroupActor = Cast<AECameraGroupActor>(GroupActorResolver->GetOwningSettingComponent()->GetAimTarget());

						TArray<FBoundingWrappedActor> FollowTargetActorsRef;
						TArray<FBoundingWrappedActor> AimTargetActorsRef;

						if (FollowGroupActor)
						{
							FollowTargetActorsRef.Append(FollowGroupActor->CameraGroupActorComponent->TargetActors);
							DrawRectOnScreenForGroupActors(FollowPositionColor, FollowTargetActorsRef);
						}
						if (AimGroupActor)
						{
							AimTargetActorsRef.Append(AimGroupActor->CameraGroupActorComponent->TargetActors);
							DrawRectOnScreenForGroupActors(AimPositionColor, AimTargetActorsRef);
						}
					}
				}
			}
		}
	}
#endif
}

void AECameraHUD::DrawRectOnScreenWithOffset(FLinearColor Color, FVector2D& ScreenOffset, FVector2D& ScreenOffsetWidth, FVector2D& ScreenOffsetHeight)
{
	int32 ScreenWidth = Canvas->SizeX;
	int32 ScreenHeight = Canvas->SizeY;
	float ScreenX = (ScreenOffset.X + 0.5f + ScreenOffsetWidth.X) * ScreenWidth;
	float ScreenY = (-ScreenOffset.Y + 0.5f - ScreenOffsetHeight.Y) * ScreenHeight;
	float ScreenW = (ScreenOffsetWidth.Y - ScreenOffsetWidth.X) * ScreenWidth;
	float ScreenH = (ScreenOffsetHeight.Y - ScreenOffsetHeight.X) * ScreenHeight;
	DrawRect(Color, ScreenX, ScreenY, ScreenW, ScreenH);
}

/** This result is not perfectly accurate, but it suffices for debug intention. */
void AECameraHUD::DrawRectOnScreenWithPosition(FLinearColor Color, FVector& Position)
{
	const float Radius = 5.0f;
	FVector2D ScreenPosition;
	UGameplayStatics::ProjectWorldToScreen(PlayerOwner, Position, ScreenPosition);
	DrawRect(Color, ScreenPosition.X - Radius, ScreenPosition.Y - Radius, 2 * Radius, 2 * Radius);
}

void AECameraHUD::DrawRectOnScreenForGroupActors(FLinearColor Color, TArray<FBoundingWrappedActor>& TargetActors)
{
	for (FBoundingWrappedActor& BWActor : TargetActors)
	{
		if (!BWActor.bExcludeBoundingBox)
		{
			const float Radius = 5.0f;
			FVector2D ScreenPosition;
			UGameplayStatics::ProjectWorldToScreen(PlayerOwner, BWActor.Target->GetActorLocation(), ScreenPosition);
			DrawRect(Color, ScreenPosition.X - Radius, ScreenPosition.Y - Radius, 2 * Radius, 2 * Radius);
			Canvas->K2_DrawBox(ScreenPosition - FVector2D(BWActor.Width, BWActor.Height), FVector2D(2 * BWActor.Width, 2 * BWActor.Height), 1.0f, Color);
		}
	}
}

void AECameraHUD::DrawRectOnScreenForAimAssist(FLinearColor Color, UControlAim* ControlAim, const FAimAssist& AimAssist)
{
	if (AimAssist.bEnableAimAssist)
	{
		for (const FOffsetActorType& OffsetTargetType : AimAssist.TargetTypes)
		{
			TArray<AActor*> OutActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), OffsetTargetType.ActorType, OutActors);
			for (AActor* TargetActor : OutActors)
			{
				FVector RealPosition = UECameraLibrary::GetPositionWithLocalOffset(TargetActor, OffsetTargetType.Offset);
				FVector LocalSpacePosition = UECameraLibrary::GetLocalSpacePosition(ControlAim->GetOwningActor(), RealPosition);

				if (LocalSpacePosition.X > 0 && LocalSpacePosition.X <= AimAssist.MaxDistance)
				{
					FVector2D ScreenPosition;
					UGameplayStatics::ProjectWorldToScreen(PlayerOwner, RealPosition, ScreenPosition);
					DrawRect(Color, ScreenPosition.X - AimAssist.MagneticRadius, ScreenPosition.Y - AimAssist.MagneticRadius, 2 * AimAssist.MagneticRadius, 2 * AimAssist.MagneticRadius);
				}
			}
		}
	}
}