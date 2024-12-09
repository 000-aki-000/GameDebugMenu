/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMPropertyJsonSystemComponent.h"

#include "GameDebugMenuFunctions.h"
#include "Component/GDMListenerComponent.h"

UGDMPropertyJsonSystemComponent::UGDMPropertyJsonSystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	RootJsonObject = MakeShared<FJsonObject>();
}

void UGDMPropertyJsonSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UGDMListenerComponent* ListenerComp = GetOwner()->GetComponentByClass<UGDMListenerComponent>();
    ListenerComp->OnChangePropertyBoolDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyBool);
    ListenerComp->OnChangePropertyIntDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyInt);
    ListenerComp->OnChangePropertyFloatDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyFloat);
    ListenerComp->OnChangePropertyByteDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyByte);
    ListenerComp->OnChangePropertyStringDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyString);
    ListenerComp->OnChangePropertyVectorDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyVector);
    ListenerComp->OnChangePropertyVector2DDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyVector2D);
    ListenerComp->OnChangePropertyRotatorDispatcher.AddUniqueDynamic(this, &UGDMPropertyJsonSystemComponent::OnChangePropertyRotator);
}

void UGDMPropertyJsonSystemComponent::AddPropertyToJsonSingle(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJsonSingle: ObjectKey is empty."));
        return;
    }

    if (!TargetObject)
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJsonSingle: TargetObject is null."));
        return;
    }

    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJsonSingle: PropertyName is empty."));
        return;
    }

    const FProperty* Property = TargetObject->GetClass()->FindPropertyByName(*PropertyName);
    if (!Property)
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJsonSingle: Property '%s' not found in object '%s'."), *PropertyName, *TargetObject->GetName());
        return;
    }

    FString PropertyValue;
    const void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);

    if (!Property->ExportText_Direct(PropertyValue, PropertyValuePtr, nullptr, TargetObject, PPF_None))
    {
        Property->ExportTextItem_Direct(PropertyValue, PropertyValuePtr, nullptr, TargetObject, PPF_None);
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(ObjectKey, ObjectJson))
    {
        /* フィールドが存在しない場合、新しいオブジェクトを作成 */
        TSharedPtr<FJsonObject> NewJsonObject = MakeShareable(new FJsonObject());
        RootJsonObject->SetObjectField(ObjectKey, NewJsonObject);
        ObjectJson = &NewJsonObject;
    }

    (*ObjectJson)->SetStringField(PropertyName, PropertyValue);

    UE_LOG(LogGDM, Log, TEXT("AddPropertyToJsonSingle: Added property '%s' with value '%s' to '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
}

void UGDMPropertyJsonSystemComponent::AddPropertyToJson(const FString& ObjectKey, UObject* TargetObject, const TArray<FString>& PropertyNames)
{
    if (ObjectKey.IsEmpty())
    {
        return;
    }
    
    if(PropertyNames.IsEmpty())
    {
        return;
    }
    
    const TSharedPtr<FJsonObject> ObjectJson = MakeShared<FJsonObject>();

    for (const FString& PropertyName : PropertyNames)
    {
        AddPropertyToJsonSingle(ObjectKey, TargetObject, PropertyName);
    }
}

void UGDMPropertyJsonSystemComponent::RemovePropertyFromJsonSingle(const FString& ObjectKey, const FString& PropertyName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJsonSingle: ObjectKey is empty."));
        return;
    }

    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJsonSingle: PropertyName is empty."));
        return;
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (RootJsonObject->TryGetObjectField(ObjectKey, ObjectJson))
    {
        if ((*ObjectJson)->HasField(PropertyName))
        {
            (*ObjectJson)->RemoveField(PropertyName);
            UE_LOG(LogGDM, Log, TEXT("RemovePropertyFromJsonSingle: Removed property '%s' from '%s'."), *PropertyName, *ObjectKey);
        }
        else
        {
            UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJsonSingle: Property '%s' not found in '%s'."), *PropertyName, *ObjectKey);
        }
    }
    else
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJsonSingle: ObjectKey '%s' not found."), *ObjectKey);
    }
}


void UGDMPropertyJsonSystemComponent::RemovePropertyToJson(const FString& ObjectKey, const TArray<FString>& PropertyNames)
{
    if (ObjectKey.IsEmpty())
    {
        return;
    }

    if (PropertyNames.IsEmpty())
    {
        /* なければフィールドごと削除 */
        RootJsonObject->RemoveField(ObjectKey);
    }
    else
    {
        const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
        if (RootJsonObject->TryGetObjectField(ObjectKey, ObjectJson))
        {
            for (const FString& PropertyName : PropertyNames)
            {
                (*ObjectJson)->RemoveField(PropertyName);
            }
        }
    }
}

void UGDMPropertyJsonSystemComponent::ApplyJsonToObject(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName)
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: ObjectKey is empty."));
        return;
    }

    if (!IsValid(TargetObject))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: TargetObject is nullptr."));
        return;
    }
    
    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: PropertyName is empty."));
        return;
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(ObjectKey, ObjectJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: ObjectKey '%s' not found in JSON."), *ObjectKey);
        return;
    }

    FString PropertyValue;
    if (!(*ObjectJson)->TryGetStringField(PropertyName, PropertyValue))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: Property '%s' not found in JSON for object '%s'."), *PropertyName, *ObjectKey);
        return;
    }

    const FProperty* Property = TargetObject->GetClass()->FindPropertyByName(*PropertyName);
    if (Property == nullptr)
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: Property '%s' not found in target object '%s'."), *PropertyName, *TargetObject->GetName());
        return;
    }

    void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    if (!Property->ImportText_Direct(*PropertyValue, PropertyValuePtr, nullptr, PPF_None))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObject: Failed to set property '%s' with value '%s' for object '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
    }
    else
    {
        UE_LOG(LogGDM, Log, TEXT("ApplyJsonToObject: Successfully set property '%s' with value '%s' for object '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
    }
}

void UGDMPropertyJsonSystemComponent::SetStringArrayToJson(const FString& Key, const TArray<FString>& StringArray)
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddStringArrayToJson: Key is empty."));
        return;
    }

    if (!RootJsonObject.IsValid())
    {
        UE_LOG(LogGDM, Error, TEXT("AddStringArrayToJson: RootJsonObject is invalid."));
        return;
    }

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    for (const FString& StringElement : StringArray)
    {
        JsonArray.Add(MakeShareable(new FJsonValueString(StringElement)));
    }

    RootJsonObject->SetArrayField(Key, JsonArray);

    UE_LOG(LogGDM, Log, TEXT("AddStringArrayToJson: Added array under key '%s'"), *Key);
}

TArray<FString> UGDMPropertyJsonSystemComponent::GetStringArrayFromJson(const FString& Key) const
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("GetStringArrayFromJson: Key is empty."));
        return TArray<FString>();
    }

    if (!RootJsonObject.IsValid())
    {
        UE_LOG(LogGDM, Error, TEXT("GetStringArrayFromJson: RootJsonObject is invalid."));
        return TArray<FString>();
    }

    const TArray<TSharedPtr<FJsonValue>>* JsonArrayPtr = nullptr;
    if (!RootJsonObject->TryGetArrayField(Key, JsonArrayPtr))
    {
        UE_LOG(LogGDM, Warning, TEXT("GetStringArrayFromJson: Key '%s' not found or not an array."), *Key);
        return TArray<FString>();
    }

    TArray<FString> StringArray;
    for (const TSharedPtr<FJsonValue>& JsonValue : *JsonArrayPtr)
    {
        if (JsonValue->Type == EJson::String)
        {
            StringArray.Add(JsonValue->AsString());
        }
        else
        {
            UE_LOG(LogGDM, Warning, TEXT("GetStringArrayFromJson: Non-string value found in array for key '%s'."), *Key);
        }
    }

    return StringArray;
}

void UGDMPropertyJsonSystemComponent::SetSingleStringToJson(const FString& Key, const FString& StringValue)
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("SetSingleStringToJson: Key is empty."));
        return;
    }

    if (!RootJsonObject.IsValid())
    {
        UE_LOG(LogGDM, Error, TEXT("SetSingleStringToJson: RootJsonObject is invalid."));
        return;
    }

    RootJsonObject->SetStringField(Key, StringValue);

    UE_LOG(LogGDM, Log, TEXT("SetSingleStringToJson: Set '%s' to key '%s'."), *StringValue, *Key);
}

FString UGDMPropertyJsonSystemComponent::GetSingleStringFromJson(const FString& Key, const FString& DefaultValue) const
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("GetSingleStringFromJson: Key is empty."));
        return DefaultValue;
    }

    if (!RootJsonObject.IsValid())
    {
        UE_LOG(LogGDM, Error, TEXT("GetSingleStringFromJson: RootJsonObject is invalid."));
        return DefaultValue;
    }

    FString StringValue;
    if (RootJsonObject->TryGetStringField(Key, StringValue))
    {
        return StringValue;
    }

    UE_LOG(LogGDM, Warning, TEXT("GetSingleStringFromJson: Key '%s' not found, returning default value."), *Key);
    return DefaultValue;
}

FString UGDMPropertyJsonSystemComponent::GetJsonAsString() const
{
    FString JsonString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    if (FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), Writer))
    {
        return JsonString;
    }
    return TEXT("");
}

bool UGDMPropertyJsonSystemComponent::BuildJsonFromString(const FString& JsonString)
{
    if (JsonString.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyJsonFromString: Input JSON string is empty."));
        return false;
    }

    TSharedPtr<FJsonObject> ParsedJsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, ParsedJsonObject) || !ParsedJsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyJsonFromString: Failed to parse JSON string."));
        return false;
    }

    RootJsonObject = ParsedJsonObject;
    UE_LOG(LogTemp, Log, TEXT("ApplyJsonFromString: Successfully updated RootJsonObject."));
    return true;
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyBool(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyInt(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyFloat(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyByte(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyString(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyVector(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyVector2D(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyRotator(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJsonSingle(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}
