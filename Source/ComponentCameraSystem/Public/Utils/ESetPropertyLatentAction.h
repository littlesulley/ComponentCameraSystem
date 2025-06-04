// Copyright 2023 by Sulley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "Utils/ECameraTypes.h"
#include "Kismet/KismetMathLibrary.h"

class FESetPropertyLatentAction : public FPendingLatentAction
{	
	template <typename PropertyType>
	struct FPropertyValuePack
	{
		using TCppType = typename PropertyType::TCppType;

		UObject* ContainerObject;
		FProperty* SourceProperty;

		TCppType SourceValue;
		TCppType TargetValue;
		TCppType* SourceValuePtr;

		void UpdateValue(const float& Progress, const float& Exp, const TEnumAsByte<EEasingFunc::Type>& Func)
		{
			if (IsValid(ContainerObject))
			{
				*SourceValuePtr = (TCppType)UKismetMathLibrary::Ease(SourceValue, TargetValue, Progress, Func, Exp);
			}
		}
	};

	typedef TArray<FPropertyValuePack<FFloatProperty>>  FFloatPropertyList;
	typedef TArray<FPropertyValuePack<FDoubleProperty>> FDoublePropertyList;
	typedef TArray<FPropertyValuePack<FUInt64Property>> FUInt64PropertyList;
	typedef TArray<FPropertyValuePack<FUInt32Property>> FUInt32PropertyList;
	typedef TArray<FPropertyValuePack<FUInt16Property>> FUInt16PropertyList;
	typedef TArray<FPropertyValuePack<FInt64Property>>  FInt64PropertyList;
	typedef TArray<FPropertyValuePack<FIntProperty>>    FIntPropertyList;
	typedef TArray<FPropertyValuePack<FInt16Property>>  FInt16PropertyList;
	typedef TArray<FPropertyValuePack<FInt8Property>>   FInt8PropertyList;
	typedef TArray<FPropertyValuePack<FByteProperty>>   FBytePropertyList;

private:
	// Container object
	UObject* Object;
	FProperty* SrcProperty;
	void* SrcPtr;

	// Blend variables
	float BlendDuration;
	TEnumAsByte<EEasingFunc::Type> BlendFunc;
	float BlendExp;
	float ElapsedTime;

	// Latent info
	ELatentOutputPins& OutPin;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	// Internal variables
	bool bInterrupted { false };
	FFloatPropertyList  FloatPropertyList{};
	FDoublePropertyList DoublePropertyList{};
	FUInt64PropertyList UInt64PropertyList{};
	FUInt32PropertyList UInt32PropertyList{};
	FUInt16PropertyList UInt16PropertyList{};
	FInt64PropertyList  Int64PropertyList{};
	FIntPropertyList    IntPropertyList{};
	FInt16PropertyList  Int16PropertyList{};
	FInt8PropertyList   Int8PropertyList{};
	FBytePropertyList   BytePropertyList{};

public:
	DECLARE_MULTICAST_DELEGATE(FOnActionCompletedOrInterrupted);
	FOnActionCompletedOrInterrupted OnActionCompletedOrInterrupted;

	FESetPropertyLatentAction(
				UObject* InObject,                           // Input object
				FProperty* InSrcProperty,                    // Input source property, can only be FNumericProperty or FStructProperty
				void* InSrcPtr,                              // Input source value address
				FProperty* InValueProperty,                  // Input target property
				void* InValuePtr,                            // Input target value address
				double InBlendDuration,                      // Blend time
				TEnumAsByte<EEasingFunc::Type> InBlendFunc,  // Blend function
				double InBlendExp,                           // Blend exponential
				ELatentOutputPins& OutPin,                   // Output execution pin
				const FLatentActionInfo& LatentInfo          // Latent info
		)
		: Object (InObject)
		, SrcProperty (InSrcProperty)
		, SrcPtr (InSrcPtr)
		, BlendDuration (InBlendDuration)
		, BlendFunc (InBlendFunc)
		, BlendExp (InBlendExp)
		, ElapsedTime (0)
		, OutPin (OutPin)
		, ExecutionFunction (LatentInfo.ExecutionFunction)
		, OutputLink (LatentInfo.Linkage)
		, CallbackTarget (LatentInfo.CallbackTarget)
	{
		if (BlendDuration <= 0)
		{
			BlendDuration = UE_KINDA_SMALL_NUMBER;
		}

		// Recursively (if Struct property) find numeric properties and store them in FPropertyValuePack
		RecursivelyFindPropertyValue(InSrcProperty, InValueProperty, InSrcPtr, InValuePtr);
	}

	virtual ~FESetPropertyLatentAction()
	{
		// @TODO: I don't know if it's required to manually delete elements in TArray in dtor
		FloatPropertyList.Empty();
		DoublePropertyList.Empty();
		UInt64PropertyList.Empty();
		UInt32PropertyList.Empty();
		UInt16PropertyList.Empty();
		Int64PropertyList.Empty();
		IntPropertyList.Empty();
		Int16PropertyList.Empty();
		Int8PropertyList.Empty();
		BytePropertyList.Empty();
	}

	void RecursivelyFindPropertyValue(FProperty* InSrcProperty, FProperty* InValueProperty, void* InSrcPtr, void* InValuePtr)
	{
		if (InSrcProperty->IsA<FStructProperty>())
		{
			const FStructProperty* SrcPropAsStruct = CastField<FStructProperty>(InSrcProperty);
			const FStructProperty* ValPropAsStruct = CastField<FStructProperty>(InValueProperty);
			if (SrcPropAsStruct->Struct == ValPropAsStruct->Struct)
			{
				for (TFieldIterator<FProperty> PropertyIt(SrcPropAsStruct->Struct); PropertyIt; ++PropertyIt)
				{
					FProperty* StructProp = *PropertyIt;
					if (StructProp->IsA<FNumericProperty>() || StructProp->IsA<FStructProperty>())
					{
						void* SubSrcPtr = StructProp->ContainerPtrToValuePtr<void>(InSrcPtr);
						void* SubValPtr = StructProp->ContainerPtrToValuePtr<void>(InValuePtr);
						RecursivelyFindPropertyValue(StructProp, StructProp, SubSrcPtr, SubValPtr);
					}
				}
			}
		}
		else if (InSrcProperty->IsA<FFloatProperty>())
		{
			FFloatProperty::TCppType SourceValue = CastField<FFloatProperty>(InSrcProperty)->GetPropertyValue(InSrcPtr);
			FFloatProperty::TCppType TargetValue 
				              = InValueProperty->IsA<FDoubleProperty>()
				              ? CastField<FDoubleProperty>(InValueProperty)->GetPropertyValue(InValuePtr)
							  : CastField<FFloatProperty>(InValueProperty)->GetPropertyValue(InValuePtr);

			FPropertyValuePack<FFloatProperty> PropertyValuePack 
			{ 
				.ContainerObject = Object, 
				.SourceProperty = InSrcProperty,
				.SourceValue = SourceValue, 
				.TargetValue = TargetValue, 
				.SourceValuePtr = (FFloatProperty::TCppType*)InSrcPtr
			};
			FloatPropertyList.Add(PropertyValuePack);
		}
		else if (InSrcProperty->IsA<FDoubleProperty>())
		{
			ConvertAndAddPropertyValuePack(CastField<FDoubleProperty>(InSrcProperty), CastField<FDoubleProperty>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FUInt64Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FUInt64Property>(InSrcProperty), CastField<FUInt64Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FUInt32Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FUInt32Property>(InSrcProperty), CastField<FUInt32Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FUInt16Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FUInt16Property>(InSrcProperty), CastField<FUInt16Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FInt64Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FInt64Property>(InSrcProperty), CastField<FInt64Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FIntProperty>())
		{
			ConvertAndAddPropertyValuePack(CastField<FIntProperty>(InSrcProperty), CastField<FIntProperty>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FInt16Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FInt16Property>(InSrcProperty), CastField<FInt16Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FInt8Property>())
		{
			ConvertAndAddPropertyValuePack(CastField<FInt8Property>(InSrcProperty), CastField<FInt8Property>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else if (InSrcProperty->IsA<FByteProperty>())
		{
			ConvertAndAddPropertyValuePack(CastField<FByteProperty>(InSrcProperty), CastField<FByteProperty>(InValueProperty), InSrcPtr, InValuePtr);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot cast propert %s to numeric or struct property tye."), *InSrcProperty->GetName());
		}
	}

	template <typename PropertyType>
	void ConvertAndAddPropertyValuePack(PropertyType* InSrcProperty, PropertyType* InValueProperty, void* InSrcPtr, void* InValuePtr)
	{
		typename PropertyType::TCppType SourceValue = InSrcProperty->GetPropertyValue(InSrcPtr);
		typename PropertyType::TCppType TargetValue = InValueProperty->GetPropertyValue(InValuePtr);
		FPropertyValuePack<PropertyType> PropertyValuePack
		{
			.ContainerObject = Object,
			.SourceProperty = InSrcProperty,
			.SourceValue = SourceValue,
			.TargetValue = TargetValue,
			.SourceValuePtr = (typename PropertyType::TCppType*)InSrcPtr
		};
		GetSpecificPropertyList(InSrcProperty).Add(PropertyValuePack);
	}

	template <typename PropertyType>
	auto&& GetSpecificPropertyList(PropertyType* Property)
	{
		if constexpr (std::is_same_v<PropertyType, FFloatProperty>)
		{
			return FloatPropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FDoubleProperty>)
		{
			return DoublePropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FUInt64Property>)
		{
			return UInt64PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FUInt32Property>)
		{
			return UInt32PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FUInt16Property>)
		{
			return UInt16PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FInt64Property>)
		{
			return Int64PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FIntProperty>)
		{
			return IntPropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FInt16Property>)
		{
			return Int16PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FInt8Property>)
		{
			return Int8PropertyList;
		}
		else if constexpr (std::is_same_v<PropertyType, FByteProperty>)
		{
			return BytePropertyList;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find matching numeric or struct property type for property %s."), *Property->GetName());
			return nullptr;
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		ElapsedTime = FMath::Clamp(ElapsedTime + Response.ElapsedTime(), 0, BlendDuration);
		bool bCompleted = ElapsedTime >= BlendDuration;

		if (bInterrupted || !IsValid(Object))
		{
			OutPin = ELatentOutputPins::OnInterrupt;
		}
		else
		{
			// On each tick, updates property values
			UpdateAllPropertyValues(ElapsedTime / BlendDuration, BlendExp, BlendFunc);

			if (bCompleted)
			{
				OutPin = ELatentOutputPins::OnComplete;
			}
			else
			{
				OutPin = ELatentOutputPins::OnTick;
			}
		}

		if (bCompleted || bInterrupted)
		{
			OnActionCompletedOrInterrupted.Broadcast();
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);

		}
		else
		{
			Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
		}
	}

	void UpdateAllPropertyValues(const float& Progress, const float& Exp, const TEnumAsByte<EEasingFunc::Type>& Func)
	{
		for (auto& PropertyValue : FloatPropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : DoublePropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : UInt64PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : UInt32PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : UInt16PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : Int64PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : IntPropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : Int16PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : Int8PropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
		for (auto& PropertyValue : BytePropertyList)
		{
			PropertyValue.UpdateValue(Progress, Exp, Func);
		}
	}

	void SetInterrupt(bool bInInterrupted)
	{
		bInterrupted = bInInterrupted;
	}

	bool IsSameProperty(FProperty* InProperty, void* InPtr)
	{
		return SrcProperty == InProperty && SrcPtr == InPtr;
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(3)
			.SetMaximumFractionalDigits(3);
		return FText::Format(NSLOCTEXT("SetPropertyLatentAction", "DelayActionTimeFmt", "Delay ({0} seconds elapsed)"), FText::AsNumber(ElapsedTime, &DelayTimeFormatOptions)).ToString();
	}
#endif
};