// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ScriptInterface.h"
#include "Templates/SubclassOf.h"
#include "Components/SceneComponent.h"
#include "Utils/ECameraTypes.h"
#include "ECameraSettingsComponent.generated.h"

class UECameraExtensionBase;
class UECameraComponentBase;
class UECameraComponentFollow;
class UECameraComponentAim;
class AECameraManager;

UCLASS(Blueprintable, BlueprintType, classGroup = "ECamera")
class COMPONENTCAMERASYSTEM_API UECameraSettingsComponent : public USceneComponent
{
	GENERATED_BODY()
	
	/** NOTE: DO NOT use DECLARE_DYNAMIC_MULTICAST_DELEGATE here!
	 *  It will bind base class's function even if it is overridden in derived class.
	 *  This will call the binding function TWICE, one for base class and one for derived class.
	 *  And the base class version will raise an error if you access the class members in the binding function.
	 */
	/** Called before any internal component is executed. */
	DECLARE_MULTICAST_DELEGATE(FOnPreTickComponent);
	/** Called after all internal components are executed. */
	DECLARE_MULTICAST_DELEGATE(FOnPostTickComponent);
	/** Called when this camera becomes view target. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnBecomeViewTarget, APlayerController*);
	/** Called when this camera ends view target. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEndViewTarget, APlayerController*);

public:
	UECameraSettingsComponent();
	friend class AECameraBase;  // Should I use friend or not?
	friend class AEKeyframeCamera;

public:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

/** Begin properties. */
protected:
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "ECamera|Follow")
	TObjectPtr<UECameraComponentFollow> FollowComponent;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "ECamera|Aim")
	TObjectPtr<UECameraComponentAim> AimComponent;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = "ECamera|Extension")
	TArray<TObjectPtr<UECameraExtensionBase>> Extensions;

protected:
	/** ComponentContainer only contains follow component and aimcomponent, excluding extensions. */
	TArray<TObjectPtr<UECameraComponentBase>> ComponentContainer;

	/** Whether to enforce active. Used for mixing camera. */
	bool bForceActive;

	UPROPERTY(BlueprintReadOnly, Category = "ECamera|FollowTarget")
	TObjectPtr<AActor> FollowTarget;

	UPROPERTY(BlueprintReadOnly, Category = "ECamera|AimTarget")
	TObjectPtr<AActor> AimTarget;

	UPROPERTY(BlueprintReadOnly, Category = "ECamera|ECameraManager")
	TObjectPtr<AECameraManager> ECameraManager;

public:
	FOnPreTickComponent OnPreTickComponent;
	FOnPostTickComponent OnPostTickComponent;
	FOnBecomeViewTarget OnBecomeViewTarget;
	FOnEndViewTarget OnEndViewTarget;

/** Begin functions. */
public:
	/** Whether current view target is self. */
	bool IsActive() const;

	/** Get player controller at player index 0. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings")
	APlayerController* GetPlayerController() const;

	/** Get owning pawn. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings")
	APawn* GetOwningPawn() const;

	/** Get owning camera actor. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings")
	AECameraBase* GetOwningCamera() const;

	/** Set follow target. Also sets FollowComponent's follow target. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	AActor* SetFollowTarget(AActor* NewFollowTarget);

	/** Set follow socket. Also sets FollowComponent's follow socket. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	FName SetFollowSocket(FName NewFollowSocket);

	/** Set follow scene component. Also sets FollowComponent's scene component. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	USceneComponent* SetFollowSceneComponent(USceneComponent* NewSceneComponent);

	/** Get player camera manager. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings")
	APlayerCameraManager* GetPlayerCameraManager() const;

	/** Set aim target. Also sets AimComponent's aim target. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	AActor* SetAimTarget(AActor* NewAimTarget);

	/** Set aim socket. Also sets AimComponent's aim socket. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	FName SetAimSocket(FName NewAimSocket);

	/** Set aim scene component. Also sets AimComponent's scene component. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	USceneComponent* SetAimSceneComponent(USceneComponent* NewSceneComponent);

	/** Get follow component of specified class. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings", meta = (DeterminesOutputType = "FollowClass"))
	UECameraComponentFollow* GetFollowComponentOfClass(TSubclassOf<UECameraComponentFollow> FollowClass);

	/** Get aim component of specified class. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings", meta = (DeterminesOutputType = "AimClass"))
	UECameraComponentAim* GetAimComponentOfClass(TSubclassOf<UECameraComponentAim> AimClass);

	/** Get extension of specified class. */
	UFUNCTION(BlueprintPure, Category = "ECamera|Settings", meta = (DeterminesOutputType = "ExtensionClass"))
	UECameraExtensionBase* GetExtensionOfClass(TSubclassOf<UECameraExtensionBase> ExtensionClass);

	/** Add an extension. If this extension already exists, won't add it. */
	UFUNCTION(BlueprintCallable, Category = "ECamera|Settings")
	void AddExtension(UECameraExtensionBase* InExtension);

	/** Get ECamera manager. */
	AECameraManager* GetECameraManager() const;

	/** Get FollowComponent. */
	UECameraComponentFollow* GetFollowComponent() const;

	/** Set FollowComponent. */
	void SetFollowComponent(UECameraComponentFollow* InFollowComponent);

	/** Get follow target. */
	AActor* GetFollowTarget() const;

	/** Get aim target. */
	AActor* GetAimTarget() const;

	/** Get AimComponent. */
	UECameraComponentAim* GetAimComponent() const;

	/** Set AimComponent. */
	void SetAimComponent(UECameraComponentAim* InAimComponent);

	/** Get Extensions. */
	TArray<UECameraExtensionBase*> GetExtensions() const;

	/** Get bForceActive. */
	bool GetForceActive() const;

	/** Set bForceActive. */
	void SetForceActive(bool InForceActive);

public:
	/** Register ECamaraManager */
	virtual void RegisterManager();
	/** Initialize all components and binds functions to delegates. */
	virtual void InitializeECameraComponents();
	virtual void InitializeECameraComponent(UECameraComponentBase* Component);

	/** What will camera do as soon as it becomes view target. */
	UFUNCTION()
	void BecomeViewTarget(APlayerController* PC, bool bPreserveState);
	/** What will camera do as soon as it exits view target. */
	UFUNCTION()
	void EndViewTarget(APlayerController* PC);

public:
#if WITH_EDITOR
	/**
	* Override PostEditChangeProperty to enable responding to property change in editor.
	* Refer to https://forums.unrealengine.com/t/how-do-i-use-posteditchangeproperty/324110 for more information.
	*/
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif	
};
