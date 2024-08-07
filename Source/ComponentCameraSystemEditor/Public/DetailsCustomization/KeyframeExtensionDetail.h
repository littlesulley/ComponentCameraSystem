// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class UActorSequenceComponent;
class UKeyframeExtension;
class AECameraBase;

class FKeyframeExtensionDetail : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    UActorSequenceComponent* GetActorSequenceComponent(AECameraBase* Camera);
    UKeyframeExtension* GetKeyframeExtension(AECameraBase* Camera);
};