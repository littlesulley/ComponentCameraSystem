// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "WaveOscillatorCameraShakePattern.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "Utils/ECameraTypes.h"
#include "Engine/Scene.h"
#include "Camera/CameraTypes.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "UObject/ScriptInterface.h"
#include "EPlayerCameraManager.generated.h"

class IBlendableInterface;

/**
 * This is the customized player camera manager. The main goal is to enable adding and clearing camera post processing effects,
 * such as depth of field, bloom, exposure, color grading, etc. Later uses, if necessary, will further extend this class.
 */
UCLASS(classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API AEPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AEPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

protected:
//  <- Post process and blendables. -> //
	FPostProcessSettings PostProcessMaterialSettings;
	TArray<FWeightedPostProcess> WeightedPostProcesses;
	TArray<FWeightedBlendableObject> WeightedBlendables;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|PostProcess")
	float MaterialWeight = 1.0f;
//  <- Post process and blendables. -> //

//  <- Screen fade. -> //
	bool bEFading = false;
	FVector2D EFadeAlpha;
	float EFadeInTime;
	float EFadeOutTime;
	float EFadeDuration;
	float EFadeTimeRemaining;
	TEnumAsByte<EEasingFunc::Type> EFadeInFunc;
	TEnumAsByte<EEasingFunc::Type> EFadeOutFunc;
//  <- Screen fade. -> //

//  <- Actor dither fade. -> //
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither")
	bool bEnableDither;

	/** A set of params defining object fade. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither", meta = (EditCondition = "bEnableDither == true"))
	FDitherParams DitherFadeParams;

	/** Maximum dither fade weight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableDither == true"))
	float MaxWeight;

	/** Fade in time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bEnableDither == true"))
	float DitherFadeInTime;

	/** Fade out time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bEnableDither == true"))
	float DitherFadeOutTime;

	/** Tick interval, in seconds. Large values save more performance but will result in lagging. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EPlayerCameraManager|Dither", meta = (EditCondition = "bEnableDither == true"))
	float DitherTickInterval;

	TMap<const AActor*, FFadedObjectInfo> DitherFadeActors;
	TMap<const APawn*, FFadedObjectInfo> DitherFadeSelf;
	float ElapsedIntervalTime;
//  <- Actor dither fade. -> //

//  <- Photo mode camera. -> //

	/** Photo mode camera class. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EPlayerCameraManager|PhotoMode")
	TSubclassOf<class AEPhotoCamera> PhotoModeCamera;

	/** Photo mode UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EPlayerCameraManager|PhotoMode")
	TSubclassOf<class UUserWidget> PhotoModeWidget;

	/** Actor types that are not paused in photo mode. Currently not working for Niagara particles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EPlayerCameraManager|PhotoMode")
	TArray<TSubclassOf<AActor>> UnpausableObjects;

	TObjectPtr<UUserWidget> PhotoModeUI = nullptr;
	TObjectPtr<class AEPhotoCamera> PhotoCamera = nullptr;
	TObjectPtr<APawn> ControlledPawn = nullptr;
	FVector PivotPosition;

	bool bOriginalEnableDither = true;

//  <- Photo mode camera. -> //

public:
	virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

protected:
	virtual void DoUpdateCamera(float DeltaTime) override;
	virtual void DoDitherFade(float DeltaTime);

public:
	/** Applies a post process material to the player camera manager. The InWeight here is different from that in AddPostProcess. This one may not perform as you expected.
	 *  @param InBlendableObject - The post process material to add.
	 *  @param InWeight - Amount of influence the post process effect will have. 1 means full effect, while 0 means no effect.
	 * 	@param InBlendInTime - Time used to blend into this post process.
	 *	@param InDuration - Duration for this post process. Set as 0 to keep it infinite.
	 *  @param InBlendOutTime - Time used to blend out of this post process.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void AddBlendable(const TScriptInterface<IBlendableInterface>& InBlendableObject, const float InWeight = 1.f, const float InBlendInTime = 0.5f, const float InDuration = 0.0f, const float InBlendOutTime = 0.5f);

	/** Removes a post process material added to the player camera manager.
	 *  @param InBlendableObject - The post process material to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void RemoveBlendable(const TScriptInterface<IBlendableInterface>& InBlendableObject);

	/** Removes all completed blendables. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void RemoveCompletedBlendables();

	/** Add a post process to the player camera manager.
	 *  @param InPostProcess - The post process setting to add.
	 *  @param InWeight - Amount of influence the post process effect will have. 1 means full effect, while 0 means no effect.
	 *  @param InBlendInTime - Time used to blend into this post process.
	 *	@param InDuration - Duration for this post process. Set as 0 to keep it infinite.
	 *  @param InBlendOutTime - Time used to blend out of this post process.
  	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void AddPostProcess(const FPostProcessSettings& InPostProcess, const float InWeight = 1.f, const float InBlendInTime = 0.5f, const float InDuration = 0.0f, const float InBlendOutTime = 0.5f);

	/** Removes all completed post processes. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void RemoveCompletedPostProcesses();

	/** Clears all post processes. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|PostProcess")
	void RemoveAllPostProcesses();

	/**
	 * Does a camera fade to a solid color and then fades back.  Animates automatically.
	 * @param FromAlpha - Alpha at which to begin the fade. Range [0..1], where 0 is fully transparent and 1 is fully opaque solid color.
	 * @param ToAlpha - Alpha at which to finish in-fade.
	 * @param FadeInTime - How long the in-fade should take, in seconds.
	 * @param FadeInFunc - Blend function for in-fade.
	 * @param Duration - How long ToAlpha remains after finishing in-fase, in seconds.
	 * @param FadeOutTime - How long the out-fade should take, in seconds.
	 * @param FadeOutFunc - Blend function for out-fade.
	 * @param Color - Color to fade to/from.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraFade", meta = (AdvancedDisplay = 2))
	virtual void EasyStartCameraFade(float FromAlpha, float ToAlpha, float FadeInTime, TEnumAsByte<EEasingFunc::Type> FadeInFunc, float Duration, float FadeOutTime, TEnumAsByte<EEasingFunc::Type> FadeOutFunc, FLinearColor Color);

	/**
	 * Stops camera fading.
	 * @param StopAlpha - Alpha at which fade stops.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraFade")
	virtual void EasyStopCameraFade(float StopAlpha = 0.0f);

	/**
	 * Immediately clear dither fading.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Dither")
	virtual void ImmediatelyClearDither();

	/** Start camera shake using Wave Oscillator pattern. 
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 * @param CameraShakeSourceComponent - The CameraShakeSourceComponent to activate.
	 * @param LocationAmplitudeMultiplier - Amplitude multiplier for all location shake.
	 * @param LocationFrequencyMultiplier - Frequency multiplier for all location shake.
	 * @param X - Shake in the X axis.
	 * @param Y - Shake in the Y axis.
	 * @param Z - Shake in the Z axis.
	 * @param RotationAmplitudeMultiplier - Amplitude multiplier for all rotation shake.
	 * @param RotationFrequencyMultiplier - Frequency multiplier for all rotation shake.
	 * @param Pitch - Pitch shake.
	 * @param Yaw - Yaw shake.
	 * @param Roll - Roll shake.
	 * @param FOV - FOV shake.
	 * @param Duration - Duration in seconds of this shake. Zero or less means infinity.
	 * @param BlendInTime - Blend-in time for this shake. Zero or less means no blend-in.
	 * @param BlendOutTime - Blend-out time for this shake. Zero or less means no blend-out.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraShake", meta = (DisplayName = "StartCameraShake(Wave)", AdvancedDisplay = 3))
	virtual UCameraShakeBase* StartCameraShakeWave(float Scale, 
												   ECameraShakePlaySpace PlaySpace, 
												   FRotator UserPlaySpaceRot, 
												   bool bSingleInstance = false,
												   UCameraShakeSourceComponent* CameraShakeSourceComponent = nullptr,
												   float LocationAmplitudeMultiplier = 1.f, 
												   float LocationFrequencyMultiplier = 1.f, 
												   FWaveOscillator X = FWaveOscillator(), 
												   FWaveOscillator Y = FWaveOscillator(), 
												   FWaveOscillator Z = FWaveOscillator(),
												   float RotationAmplitudeMultiplier = 1.f, 
												   float RotationFrequencyMultiplier = 1.f, 
												   FWaveOscillator Pitch = FWaveOscillator(), 
												   FWaveOscillator Yaw = FWaveOscillator(), 
												   FWaveOscillator Roll = FWaveOscillator(), 
												   FWaveOscillator FOV = FWaveOscillator(),
												   float Duration = 1.f, 
												   float BlendInTime = 0.2f, 
												   float BlendOutTime = 0.2f);

	/** Start camera shake using Perlin Noise pattern. 
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 * @param CameraShakeSourceComponent - The optional CameraShakeSourceComponent to activate.
	 * @param LocationAmplitudeMultiplier - Amplitude multiplier for all location shake.
	 * @param LocationFrequencyMultiplier - Frequency multiplier for all location shake.
	 * @param X - Shake in the X axis.
	 * @param Y - Shake in the Y axis.
	 * @param Z - Shake in the Z axis.
	 * @param RotationAmplitudeMultiplier - Amplitude multiplier for all rotation shake.
	 * @param RotationFrequencyMultiplier - Frequency multiplier for all rotation shake.
	 * @param Pitch - Pitch shake.
	 * @param Yaw - Yaw shake.
	 * @param Roll - Roll shake.
	 * @param FOV - FOV shake.
	 * @param Duration - Duration in seconds of this shake. Zero or less means infinity.
	 * @param BlendInTime - Blend-in time for this shake. Zero or less means no blend-in.
	 * @param BlendOutTime - Blend-out time for this shake. Zero or less means no blend-out.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraShake", meta = (DisplayName = "StartCameraShake(Perlin)", AdvancedDisplay = 3))
	virtual UCameraShakeBase* StartCameraShakePerlin(float Scale,
												 	 ECameraShakePlaySpace PlaySpace,
													 FRotator UserPlaySpaceRot,
													 bool bSingleInstance = false,
													 UCameraShakeSourceComponent* CameraShakeSourceComponent = nullptr,
													 float LocationAmplitudeMultiplier = 1.f,
													 float LocationFrequencyMultiplier = 1.f,
													 FPerlinNoiseShaker X = FPerlinNoiseShaker(),
													 FPerlinNoiseShaker Y = FPerlinNoiseShaker(),
													 FPerlinNoiseShaker Z = FPerlinNoiseShaker(),
													 float RotationAmplitudeMultiplier = 1.f,
													 float RotationFrequencyMultiplier = 1.f,
													 FPerlinNoiseShaker Pitch = FPerlinNoiseShaker(),
													 FPerlinNoiseShaker Yaw = FPerlinNoiseShaker(),
													 FPerlinNoiseShaker Roll = FPerlinNoiseShaker(),
													 FPerlinNoiseShaker FOV = FPerlinNoiseShaker(),
													 float Duration = 1.f,
													 float BlendInTime = 0.2f,
													 float BlendOutTime = 0.2f);

	/** Start camera shake with shake class and optional packed oscillation parameters.
	 * @param ShakeClass - The class of camera shake to play.
	 * @param ShakeParams - Packed oscillation parameters. If ShakeClass is not specified, this parameter will be used.
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 * @param CameraShakeSourceComponent - The optional CameraShakeSourceComponent to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraShake", meta = (DisplayName = "EasyStartCameraShake", AdvancedDisplay = 3))
	virtual UCameraShakeBase* EasyStartCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, 
												   FPackedOscillationParams ShakeParams = FPackedOscillationParams(), 
												   float Scale = 1.0, 
												   ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal, 
												   FRotator UserPlaySpaceRot = FRotator(0, 0, 0), 
												   bool bSingleInstance = false,
												   UCameraShakeSourceComponent * CameraShakeSourceComponent = nullptr);

	/** Start camera shake with shake class and optional packed oscillation parameters. This function is intended for temoprary camera shake, 
	 * i.e., camera shakes with finite duration. If you are using infinite camera shakes, though you can still specify an inifite time
	 * in this node, it's highly recommended to use UE's built-in `Start Camera Shake From Source` nodes and `Stop Camera Shake` to 
	 * terminate specific camera shake instances.
	 * @param ShakeClass - The class of camera shake to play.
	 * @param ShakeParams - Packed oscillation parameters. If ShakeClass is not specified, this parameter will be used.
	 * @param SpawnActor - Actor to which the camera shake source actor will be spawned and attached.
	 * @param SpawnLocation - Used when SpawnActor is null. Location where the camera shake source actor will be spawned.
	 * @param Attenuation - The attenuation profile for how camera shakes' intensity falls off with distance.
	 * @param InnerAttenuationRadius - Under this distance from the source, the camera shakes are at full intensity.
	 * @param OuterAttenuationRadius - Outside of this distance from the source, the cmaera shakes don't apply at all.
	 * @param Scale - Scalar defining how intense to play the shake. 1.0 is normal.
	 * @param PlaySpace - Which coordinate system to play the shake in (affects oscillations and camera anims).
	 * @param UserPlaySpaceRot - Coordinate system to play shake when PlaySpace == UserDefined.
	 * @param bSingleInstance - If true, only allow a single instance of this shake class to play at each time. Subsequent attempts to play this shake will simply restart the timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "ECamera|CameraShake", meta = (DisplayName = "EasyStartCameraShakeFromSource", AdvancedDisplay = 4))
	virtual UCameraShakeBase* EasyStartCameraShakeFromSource(TSubclassOf<UCameraShakeBase> ShakeClass,
															 FPackedOscillationParams ShakeParams = FPackedOscillationParams(),
															 AActor* SpawnActor = nullptr,
															 FVector SpawnLocation = FVector::ZeroVector,
															 ECameraShakeAttenuation Attenuation = ECameraShakeAttenuation::Linear,
															 float InnerAttenuationRadius = 0.0f,
															 float OuterAttenuationRadius = 1000.0f,
															 float Scale = 1.0,
															 ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal,
															 FRotator UserPlaySpaceRot = FRotator(0, 0, 0),
															 bool bSingleInstance = false);
	
	UFUNCTION(BlueprintCallable, Category = "ECamera|PhotoMode")
	void SwitchPhotoMode();

private:
	float GetBlendedWeight(const float& StartWeight, const float& TargetWeight, const float& BlendTime, const float& ElapsedTime);
	float GetPPWeight(float& Weight, float& BlendInTime, float& Duration, float& BlendOutTime, float& ElapsedBlendInTime, float& ElapsedDurationTime, float& ElapsedBlendOutTime, bool& bHasCompleted, const float& DeltaTime);
	
	TArray<FHitResult> DoDitherCollisionCheck(AActor* Target, float MaxLength, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes);
	void UpdateDitherWithFadeOut(float DeltaTime);
	void UpdateDitherWithDefault(float DeltaTime);
	void UpdateDitherWithNew(float DeltaTime, TArray<FHitResult>& OutHits);
	void UpdateSelfDitherWithDefault(float DeltaTime);
	void UpdateSelfDitherWithNew(float DeltaTime, TArray<FHitResult>& OutHits);
	void UpdateDitherWeightSingle(const AActor* Actor, FFadedObjectInfo& FadeInfo);
	void UpdateDitherWeight(TMap<const AActor*, FFadedObjectInfo> FadeActors);
	void UpdateDitherWeight(TMap<const APawn*, FFadedObjectInfo> FadeSelf);
	void OverrideDitherMaterials(const AActor* Actor);
	void OverrideDitherMaterials(const APawn* Pawn);
	void RestoreDitherMaterials(TArray<const AActor*> Actors);
	void RestoreDitherMaterials(TArray<const APawn*> Pawns);

public:
	UUserWidget* GetPhotoModeUI() { return PhotoModeUI; }
	FVector GetPhotoModePivotPosition() { return PivotPosition; }
	APawn* GetPhotoModeControlledPawn() { return ControlledPawn; }
	void PauseGame();
	void UnpauseGame();
};
