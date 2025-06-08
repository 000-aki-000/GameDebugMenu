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

	/* 無操作状態からリセットまでの猶予（秒） */
	constexpr float InactiveResetThreshold = 0.18f; 
	
	if (bStartChangeAmount)
	{
		/* 変化量の時間計測 */
		if (!bChangedMaxChangeAmount)
		{
			ElapsedTime -= InDeltaTime;
			if (ElapsedTime <= 0.0f)
			{
				ElapsedTime = 0.0f;
				ChangeAmount = PropertyConfigInfo.MaxChangeAmount;
				bChangedMaxChangeAmount = true;
			}
		}

		InactiveElapsedTime += InDeltaTime;
		if (InactiveElapsedTime >= InactiveResetThreshold)
		{
			/* 一定時間StartChangeAmountTimeが呼ばれなくなったのでもう数値が変動してないと判断し戻す */
			ResetChangeAmountTime();
		}
	}
}

void UGDMPropertyWidget::StartChangeAmountTime()
{
	if (!bStartChangeAmount)
	{
		bStartChangeAmount = true;
		bChangedMaxChangeAmount = false;
		ChangeAmount = PropertyConfigInfo.DefaultChangeAmount;
		ElapsedTime = PropertyConfigInfo.MaxChangeAmountTime;
	}

	/* 呼ばれるたびに無操作時間をリセット */ 
	InactiveElapsedTime = 0.0f;
}

void UGDMPropertyWidget::ResetChangeAmountTime()
{
	if (!bStartChangeAmount)
	{
		return;
	}

	bStartChangeAmount = false;
	bChangedMaxChangeAmount = false;
	ChangeAmount = PropertyConfigInfo.DefaultChangeAmount;
	InactiveElapsedTime = 0.0f;
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

	const bool bOldValue = BoolProp->GetPropertyValue(BoolProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (bNewValue != bOldValue)
	{
		BoolProp->SetPropertyValue(BoolProp->ContainerPtrToValuePtr<void*>(TargetObject), bNewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyBoolDispatcher(PropertyName, TargetObject, bNewValue, bOldValue, PropertySaveKey);
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
		if(const FDoubleProperty* DoubleProp = CastField<const FDoubleProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName)))
		{
			bHasProperty = true;

			return DoubleProp->GetPropertyValue(DoubleProp->ContainerPtrToValuePtr<void*>(TargetObject));
		}
		
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
		if(const FDoubleProperty* DoubleProp = CastField<const FDoubleProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName)))
		{
			bHasProperty = true;

			const float OldValue = DoubleProp->GetPropertyValue(DoubleProp->ContainerPtrToValuePtr<void*>(TargetObject));
			if (FMath::IsNearlyEqual(NewValue, OldValue) == false)
			{
				DoubleProp->SetPropertyValue(DoubleProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
				UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyFloatDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
			}
		}
		return;
	}

	bHasProperty = true;

	const float OldValue = FloatProp->GetPropertyValue(FloatProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (FMath::IsNearlyEqual(NewValue, OldValue) == false)
	{
		FloatProp->SetPropertyValue(FloatProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyFloatDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
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

	const int32 OldValue = IntProp->GetPropertyValue(IntProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (NewValue != OldValue)
	{
		IntProp->SetPropertyValue(IntProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyIntDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
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
		const uint64 Value = NumProp->GetUnsignedIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject));
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

		const FNumericProperty* NumProp = EnumProp->GetUnderlyingProperty();
		const uint8 OldValue = static_cast<uint8>( NumProp->GetUnsignedIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject)) );
		if( NewValue != OldValue )
		{
			NumProp->SetIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject),static_cast<uint64>( NewValue ));
			UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyByteDispatcher(PropertyName,TargetObject,NewValue,OldValue, PropertySaveKey);
		}
		return;
	}

	const FByteProperty* ByteProp = CastField<const FByteProperty>(Prop);
	if( ByteProp != nullptr )
	{
		bHasProperty = true;

		const uint8 OldValue = ByteProp->GetPropertyValue(ByteProp->ContainerPtrToValuePtr<void*>(TargetObject));
		if( NewValue != OldValue )
		{
			ByteProp->SetPropertyValue(ByteProp->ContainerPtrToValuePtr<void*>(TargetObject),NewValue);
			UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyByteDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
		}
	}
}

TArray<FText> UGDMPropertyWidget::GetEnumDisplayNames(const FString& EnumPath, bool& bHasProperty)
{
	TArray<FText> Result;

	const UEnum* Enum = FindObject<UEnum>(nullptr, *EnumPath);
	if(!IsValid(Enum))
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

	const FString OldValue = StrProp->GetPropertyValue(StrProp->ContainerPtrToValuePtr<void*>(TargetObject));
	if (NewValue != OldValue)
	{
		StrProp->SetPropertyValue(StrProp->ContainerPtrToValuePtr<void*>(TargetObject), NewValue);
		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyStringDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
	}
}

FVector UGDMPropertyWidget::GetPropertyValue_Vector(bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return FVector::ZeroVector;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return FVector::ZeroVector;
	}

	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return FVector::ZeroVector;
	}

	if( Struct->IsChildOf(TBaseStructure<FVector>::Get()) )
	{
		if(const FVector* Value = StructProp->ContainerPtrToValuePtr<FVector>(TargetObject) )
		{
			bHasProperty = true;
			return (*Value);
		}
	}

	return FVector::ZeroVector;
}

void UGDMPropertyWidget::SetPropertyValue_Vector(FVector NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return;
	}
 
	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return;
	}

	if( Struct->IsChildOf(TBaseStructure<FVector>::Get()) )
	{
		if( FVector* Value = StructProp->ContainerPtrToValuePtr<FVector>(TargetObject) )
		{
			const FVector OldValue = *Value;

			bHasProperty = true;

			if( NewValue != OldValue )
			{
				*Value = NewValue;
				UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyVectorDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
			}
		}
	}
}

FVector2D UGDMPropertyWidget::GetPropertyValue_Vector2D(bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return FVector2D::ZeroVector;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return FVector2D::ZeroVector;
	}

	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return FVector2D::ZeroVector;
	}

	if( Struct->IsChildOf(TBaseStructure<FVector2D>::Get()) )
	{
		if(const FVector2D* Value = StructProp->ContainerPtrToValuePtr<FVector2D>(TargetObject) )
		{
			bHasProperty = true;
			return (*Value);
		}
	}

	return FVector2D::ZeroVector;
}

void UGDMPropertyWidget::SetPropertyValue_Vector2D(FVector2D NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return;
	}

	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return;
	}

	if( Struct->IsChildOf(TBaseStructure<FVector2D>::Get()) )
	{
		if( FVector2D* Value = StructProp->ContainerPtrToValuePtr<FVector2D>(TargetObject) )
		{
			const FVector2D OldValue = *Value;

			bHasProperty = true;

			if( NewValue != OldValue )
			{
				*Value = NewValue;
				UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyVector2DDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
			}
		}
	}
}

FRotator UGDMPropertyWidget::GetPropertyValue_Rotator(bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return FRotator::ZeroRotator;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return FRotator::ZeroRotator;
	}

	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return FRotator::ZeroRotator;
	}

	if( Struct->IsChildOf(TBaseStructure<FRotator>::Get()) )
	{
		if(const FRotator* Value = StructProp->ContainerPtrToValuePtr<FRotator>(TargetObject) )
		{
			bHasProperty = true;
			return (*Value);
		}
	}

	return FRotator::ZeroRotator;
}

void UGDMPropertyWidget::SetPropertyValue_Rotator(FRotator NewValue, bool& bHasProperty)
{
	bHasProperty = false;

	if( !IsValid(TargetObject) )
	{
		return;
	}

	const FStructProperty* StructProp = CastField<const FStructProperty>(TargetObject->GetClass()->FindPropertyByName(PropertyName));
	if( StructProp == nullptr )
	{
		return;
	}

	const UStruct* Struct = StructProp->Struct;
	if( Struct == nullptr )
	{
		return;
	}

	if( Struct->IsChildOf(TBaseStructure<FRotator>::Get()) )
	{
		if( FRotator* Value = StructProp->ContainerPtrToValuePtr<FRotator>(TargetObject) )
		{
			const FRotator OldValue = *Value;

			bHasProperty = true;

			if( NewValue != OldValue )
			{
				*Value = NewValue;
				UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallChangePropertyRotatorDispatcher(PropertyName, TargetObject, NewValue, OldValue, PropertySaveKey);
			}
		}
	}
}
