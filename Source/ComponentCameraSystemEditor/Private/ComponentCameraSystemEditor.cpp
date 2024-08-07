// Copyright 2023 by Sulley. All Rights Reserved.

#include "ComponentCameraSystemEditor.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "DetailsCustomization/KeyframeExtensionDetail.h"
#include "DetailsCustomization/CameraBaseDetail.h"
#include "Core/ECameraBase.h"

#define LOCTEXT_NAMESPACE "FComponentCameraSystemEditorModule"

void FComponentCameraSystemEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(AECameraBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FKeyframeExtensionDetail::MakeInstance));
    PropertyModule.RegisterCustomClassLayout(AECameraBase::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FCameraBaseDetail::MakeInstance));
    PropertyModule.NotifyCustomizationModuleChanged();
}

void FComponentCameraSystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout(AECameraBase::StaticClass()->GetFName());
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FComponentCameraSystemEditorModule, ComponentCameraSystemEditor)