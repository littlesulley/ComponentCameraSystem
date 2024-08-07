// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class AECameraBase;

class FCameraBaseDetail : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnButtonClicked(AECameraBase* Camera);
	FText OnGetButtonText() const;
	FText OnGetButtonTooltipText() const;

	void ForEachPropertyToDuplicateInstancedObjects(UObject* SourceObject, UObject* TargetObject);
	void RecursivelyDuplicateInstancedObjects(void* SourceAddress, void* TargetAddress, FProperty* ParentProperty);
};