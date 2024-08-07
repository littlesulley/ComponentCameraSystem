// Copyright 2023 by Sulley. All Rights Reserved.

#include "DetailsCustomization/CameraBaseDetail.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/SCS_Node.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Internationalization/Internationalization.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "Components/ScreenFollow.h"
#include "Components/SimpleFollow.h"
#include "Components/ECameraComponentFollow.h"
#include "Components/ECameraComponentAim.h"
#include "Core/ECameraBase.h"
#include "Core/ECameraSettingsComponent.h"
#include "Core/ECameraSubsystem.h"

#define LOCTEXT_NAMESPACE "CameraBaseDetail"

TSharedRef<IDetailCustomization> FCameraBaseDetail::MakeInstance()
{
	return MakeShareable(new FCameraBaseDetail);
}

void FCameraBaseDetail::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}

	AECameraBase* Camera = Cast<AECameraBase>(Objects[0].Get());
	
	auto& Category = DetailBuilder.EditCategory("ECamera Actions");
	Category.AddCustomRow(LOCTEXT("ApplyChangesToBlueprint_Filter", "Apply Changes to Blueprint"))
		[
			SNew(SButton)
			.Text(this, &FCameraBaseDetail::OnGetButtonText)
			.ToolTipText(this, &FCameraBaseDetail::OnGetButtonTooltipText)
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([this, Camera]() {return OnButtonClicked(Camera); })
		];
}

struct FRestoreActiveCameraAfterReinstance
{
	TWeakObjectPtr<UClass> ActorClass;
	FName ActorName;
	TWeakObjectPtr<UObject> ActorOuter;
	TWeakObjectPtr<AActor> FollowActor;
	TWeakObjectPtr<AActor> AimActor;

	FRestoreActiveCameraAfterReinstance()
		: ActorClass(nullptr)
		, ActorOuter(nullptr)
	{ }

	void Save(AECameraBase* InCamera)
	{
		check(InCamera);
		ActorClass = InCamera->GetClass();
		ActorName = InCamera->GetFName();
		ActorOuter = InCamera->GetOuter();
		FollowActor = InCamera->GetFollowTarget();
		AimActor = InCamera->GetAimTarget();
	}

	void Restore()
	{
		AECameraBase* Camera = (ActorClass.IsValid() && ActorOuter.IsValid()) ? Cast<AECameraBase>((UObject*)FindObjectWithOuter(ActorOuter.Get(), ActorClass.Get(), ActorName)) : nullptr;
		if (Camera)
		{
			if (Camera->GetFollowComponent())
			{
				Camera->GetFollowComponent()->SetFollowTarget(FollowActor.Get());
			}
			if (Camera->GetAimComponent())
			{
				Camera->GetAimComponent()->SetAimTarget(AimActor.Get());
			}

			if (const UWorld* World = Camera->GetWorld())
			{
				if (const UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UECameraSubsystem* Subsystem = GameInstance->GetSubsystem<UECameraSubsystem>())
					{
						if (IsValid(Subsystem))
						{
							Subsystem->AddCamera(Camera);
							Subsystem->RefreshContainer();
						}
					}
				}
			}
		}
	}
};

FReply FCameraBaseDetail::OnButtonClicked(AECameraBase* Camera)
{
	FReply Handled = FReply::Unhandled();

	if (IsValid(Camera))
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(Camera->GetClass()->ClassGeneratedBy);
		if (Blueprint)
		{
			int NumChangedProperties = 0;
			FRestoreActiveCameraAfterReinstance RestoreActiveCameraAfterReinstance;

			AECameraBase* CDO = Camera->GetClass()->GetDefaultObject<AECameraBase>();
			UClass* Class = CDO->GetClass();
			
			Camera->Modify();
			for (UActorComponent* ActorComponent : Camera->GetComponents())
			{
				if (ActorComponent && (ActorComponent->CreationMethod == EComponentCreationMethod::SimpleConstructionScript || ActorComponent->CreationMethod == EComponentCreationMethod::Native))
				{
					ActorComponent->Modify();
				}
			}

			{
				// Save actor info so that it can be restored as active camera after reinstancing
				RestoreActiveCameraAfterReinstance.Save(Camera);

				// Apply changes to CDO except Follow/Aim/Extensions in ECameraSettingsComponent
				const EditorUtilities::ECopyOptions::Type CopyOptions = (EditorUtilities::ECopyOptions::Type)(EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties | EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
				NumChangedProperties = EditorUtilities::CopyActorProperties(Camera, CDO, CopyOptions);
				
				// Apply changes to CDO for Follow/Aim/Extensions, i.e., the instanced properties in ECameraSettingsComponent
				// @TODO: CopyPropertiesForUnrelatedObjects is UE's built-in function. Should investigate more to avoid potential glitches
				UECameraSettingsComponent* SourceComponent = Cast<UECameraSettingsComponent>(Camera->GetComponentByClass(UECameraSettingsComponent::StaticClass()));
				UECameraSettingsComponent* TargetComponent = Cast<UECameraSettingsComponent>(CDO->GetComponentByClass(UECameraSettingsComponent::StaticClass()));
				UEditorEngine::FCopyPropertiesForUnrelatedObjectsParams CopyParams;
				UEditorEngine::CopyPropertiesForUnrelatedObjects(SourceComponent, TargetComponent, CopyParams);
				TargetComponent->PostReinitProperties();
			}

			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			FKismetEditorUtilities::CompileBlueprint(Blueprint);
			RestoreActiveCameraAfterReinstance.Restore();
			Handled = FReply::Handled();
		}
	}

	return Handled;
}

void FCameraBaseDetail::ForEachPropertyToDuplicateInstancedObjects(UObject* SourceObject, UObject* TargetObject)
{
	if (SourceObject && TargetObject)
	{
		for (FProperty* Property = SourceObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			void* SourceAddress = Property->ContainerPtrToValuePtr<void>(SourceObject);
			void* TargetAddress = Property->ContainerPtrToValuePtr<void>(TargetObject);
			RecursivelyDuplicateInstancedObjects(SourceAddress, TargetAddress, Property);
		}
	}
}

// Reference: InstancedReferenceSubobjectHelper.cpp
void FCameraBaseDetail::RecursivelyDuplicateInstancedObjects(void* SourceAddress, void* TargetAddress, FProperty* ThisProperty)
{
	if (const FStructProperty* StructProperty = CastField<const FStructProperty>(ThisProperty))
	{
		if (!StructProperty->HasAnyPropertyFlags(CPF_ContainsInstancedReference) || !StructProperty->Struct)
		{
			return;
		}

		for (FProperty* Property = StructProperty->Struct->RefLink; Property; Property = Property->NextRef)
		{
			void* SourceValueAddress = Property->ContainerPtrToValuePtr<void>(SourceAddress);
			void* TargetValueAddress = Property->ContainerPtrToValuePtr<void>(TargetAddress);
			RecursivelyDuplicateInstancedObjects(SourceValueAddress, TargetValueAddress, Property);
		}
	}
	else if (const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(ThisProperty))
	{
		if (!ArrayProperty->HasAnyPropertyFlags(CPF_ContainsInstancedReference))
		{
			return;
		}

		// TODO...
	}
}

FText FCameraBaseDetail::OnGetButtonText() const
{
	return LOCTEXT("ApplyInstanceChangesToBlueprint", "Apply Instance Changes to Blueprint");
}

FText FCameraBaseDetail::OnGetButtonTooltipText() const
{
	return LOCTEXT("ApplyInstanceChangesToBlueprintTooltip", "Click this button to apply this instance's changes to the blueprint.");
}

#undef LOCTEXT_NAMESPACE