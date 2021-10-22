/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMPropertyWidget.h"
#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuTypes.h"

void UGDMPropertyWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGDMPropertyWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGDMPropertyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(bStartChangeAmount)
	{
		if(!bChangedMaxChangeAmount)
		{
			ElapsedTime -= InDeltaTime;

			if(ElapsedTime <= 0.0f)
			{
				ElapsedTime = 0.0f;
				ChangeAmount = PropertyConfigInfo.MaxChangeAmount;
				bChangedMaxChangeAmount = true;
			}
		}
	}
}

void UGDMPropertyWidget::StartChangeAmountTime()
{
	if(bStartChangeAmount)
	{
		return;
	}

	bStartChangeAmount      = true;
	bChangedMaxChangeAmount = false;
	ChangeAmount            = PropertyConfigInfo.DefaultChangeAmount;
	ElapsedTime             = PropertyConfigInfo.MaxChangeAmountTime;
}

void UGDMPropertyWidget::ResetChangeAmountTime()
{
	if(!bStartChangeAmount)
	{
		return;
	}

	bStartChangeAmount      = false;
	bChangedMaxChangeAmount = false;
	ChangeAmount            = PropertyConfigInfo.DefaultChangeAmount;
}

bool UGDMPropertyWidget::GetPropertyValue_Bool(bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return false;
	}

	const FBoolProperty* BoolProp = CastField<const FBoolProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(BoolProp == nullptr)
	{
		return false;
	}

	bHasProperty = true;

	return BoolProp->GetPropertyValue(BoolProp->ContainerPtrToValuePtr<void*>(TargetObject));
}

void UGDMPropertyWidget::SetPropertyValue_Bool(bool bNewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return;
	}
	const FBoolProperty* BoolProp = CastField<const FBoolProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(BoolProp == nullptr)
	{
		return;
	}

	bHasProperty = true;

	bool bOldValue = BoolProp->GetPropertyValue(BoolProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (bNewValue != bOldValue)
	{
		BoolProp->SetPropertyValue(BoolProp->ContainerPtrToValuePtr<void*>(TargetObject), bNewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyBoolDispatcher(PropertyName, TargetObject, bNewValue, bOldValue);
	}
}

float UGDMPropertyWidget::GetPropertyValue_Float(bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return 0.0f;
	}
	const FFloatProperty* FloatProp = CastField<const FFloatProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(FloatProp == nullptr)
	{
		return 0.0f;
	}

	bHasProperty = true;

	return FloatProp->GetPropertyValue(FloatProp->ContainerPtrToValuePtr<void*>(TargetObject));
}

void UGDMPropertyWidget::SetPropertyValue_Float(float NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return;
	}
	const FFloatProperty* FloatProp = CastField<const FFloatProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(FloatProp == nullptr)
	{
		return;
	}

	bHasProperty = true;

	float OldValue = FloatProp->GetPropertyValue(FloatProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (FMath::IsNearlyEqual(NewValue, OldValue) == false)
	{
		FloatProp->SetPropertyValue(FloatProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyFloatDispatcher(PropertyName, TargetObject, NewValue, OldValue);
	}
}

int32 UGDMPropertyWidget::GetPropertyValue_Int(bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return 0;
	}
	const FIntProperty* IntProp = CastField<const FIntProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(IntProp == nullptr)
	{
		return 0;
	}

	bHasProperty = true;

	return IntProp->GetPropertyValue(IntProp->ContainerPtrToValuePtr<void*>(TargetObject));
}

void UGDMPropertyWidget::SetPropertyValue_Int(int32 NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return;
	}
	const FIntProperty* IntProp = CastField<const FIntProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if(IntProp == nullptr)
	{
		return;
	}

	bHasProperty = true;

	int32 OldValue = IntProp->GetPropertyValue(IntProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (NewValue != OldValue)
	{
		IntProp->SetPropertyValue(IntProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyIntDispatcher(PropertyName, TargetObject, NewValue, OldValue);
	}
}

uint8 UGDMPropertyWidget::GetPropertyValue_Byte(bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return 0;
	}

	const FProperty* Prop = TargetObject->GetClass()->FindPropertyByName(PropertyName);
	if( Prop == nullptr )
	{
		return 0;
	}

	const FEnumProperty* EnumProp = CastField<const FEnumProperty>(Prop);
	if( EnumProp != nullptr )
	{
		bHasProperty = true;
		const FNumericProperty* NumProp = EnumProp->GetUnderlyingProperty();
		uint64 Value = NumProp->GetUnsignedIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject));
		return static_cast<uint8>( Value );
	}

	const FByteProperty* ByteProp = CastField<const FByteProperty>(Prop);
	if( ByteProp != nullptr )
	{
		bHasProperty = true;
		return ByteProp->GetPropertyValue(ByteProp->ContainerPtrToValuePtr<void*>(TargetObject));
	}
	return 0;
}

void UGDMPropertyWidget::SetPropertyValue_Byte(uint8 NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return;
	}

	const FProperty* Prop = TargetObject->GetClass()->FindPropertyByName(PropertyName);
	if( Prop == nullptr )
	{
		return;
	}

	const FEnumProperty* EnumProp = CastField<const FEnumProperty>(Prop);
	if( EnumProp != nullptr )
	{
		bHasProperty = true;

		FNumericProperty* NumProp = EnumProp->GetUnderlyingProperty();
		uint8 OldValue = static_cast<uint8>( NumProp->GetUnsignedIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject)) );
		if( NewValue != OldValue )
		{
			NumProp->SetIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject),static_cast<uint64>( NewValue ));
			UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyByteDispatcher(PropertyName,TargetObject,NewValue,OldValue);
		}
		return;
	}

	const FByteProperty* ByteProp = CastField<const FByteProperty>(Prop);
	if( ByteProp != nullptr )
	{
		bHasProperty = true;

		uint8 OldValue = ByteProp->GetPropertyValue(ByteProp->ContainerPtrToValuePtr<void*>(TargetObject));
		if( NewValue != OldValue )
		{
			ByteProp->SetPropertyValue(ByteProp->ContainerPtrToValuePtr<void*>(TargetObject),NewValue);
			UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyByteDispatcher(PropertyName,TargetObject,NewValue,OldValue);
		}
	}
}

TArray<FText> UGDMPropertyWidget::GetEnumDisplayNames(bool& bHasProperty)
{
	TArray<FText> Result;

	const UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName.ToString());
	if(IsValid(Enum) == false)
	{
		return Result;
	}

	const int32 Num = Enum->NumEnums() - 1;
	for(int32 Index = 0;Index < Num; ++Index)
	{
		Result.Add(Enum->GetDisplayNameTextByIndex(Index));
	}

	bHasProperty = true;
	return Result;
}

FString UGDMPropertyWidget::GetPropertyValue_String(bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return FString();
	}
	const FStrProperty* StrProp = CastField<const FStrProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if (StrProp == nullptr)
	{
		return FString();
	}

	bHasProperty = true;

	return StrProp->GetPropertyValue(StrProp->ContainerPtrToValuePtr<void*>(TargetObject));
}

void UGDMPropertyWidget::SetPropertyValue_String(FString NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if(!IsValid(TargetObject))
	{
		return;
	}
	const FStrProperty* StrProp = CastField<const FStrProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if (StrProp == nullptr)
	{
		return;
	}

	bHasProperty = true;

	FString OldValue = StrProp->GetPropertyValue(StrProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (NewValue != OldValue)
	{
		StrProp->SetPropertyValue(StrProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyStringDispatcher(PropertyName, TargetObject, NewValue, OldValue);
	}
}
