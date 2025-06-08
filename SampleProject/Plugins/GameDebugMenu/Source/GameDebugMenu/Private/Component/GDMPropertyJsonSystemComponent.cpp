/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMPropertyJsonSystemComponent.h"

#include "GameDebugMenuFunctions.h"
#include "Component/GDMListenerComponent.h"

const FString UGDMPropertyJsonSystemComponent::JsonField_RootProperty(TEXT("Properties"));
const FString UGDMPropertyJsonSystemComponent::JsonField_RootFunction(TEXT("Functions"));
const FString UGDMPropertyJsonSystemComponent::JsonField_RootCustom(TEXT("Custom"));
const FString UGDMPropertyJsonSystemComponent::JsonField_RootFavorite(TEXT("Favorites"));
const FString UGDMPropertyJsonSystemComponent::JsonField_FavoriteDefinitionName(TEXT("DefinitionName"));
const FString UGDMPropertyJsonSystemComponent::JsonField_FavoriteSaveKey(TEXT("SaveKey"));

UGDMPropertyJsonSystemComponent::UGDMPropertyJsonSystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	RootJsonObject = MakeShared<FJsonObject>();
}

void UGDMPropertyJsonSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    /* OwnerはAGameDebugMenuManagerであること前提 */
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

void UGDMPropertyJsonSystemComponent::AddPropertyToJson(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJson: ObjectKey is empty."));
        return;
    }

    if (!IsValid(TargetObject))
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJson: TargetObject is null."));
        return;
    }

    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJson: PropertyName is empty."));
        return;
    }

    const FProperty* Property = TargetObject->GetClass()->FindPropertyByName(*PropertyName);
    if (!Property)
    {
        UE_LOG(LogGDM, Warning, TEXT("AddPropertyToJson: Property '%s' not found in object '%s'."), *PropertyName, *TargetObject->GetName());
        return;
    }

    FString PropertyValue;
    const void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);

    if (!Property->ExportText_Direct(PropertyValue, PropertyValuePtr, nullptr, TargetObject, PPF_None))
    {
        Property->ExportTextItem_Direct(PropertyValue, PropertyValuePtr, nullptr, TargetObject, PPF_None);
    }
    
    const TSharedPtr<FJsonObject>* RootPropertyJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootProperty, RootPropertyJson))
    {
        /* フィールドが存在しない場合、新しいオブジェクトを作成 */
        TSharedPtr<FJsonObject> NewJsonObject = MakeShareable(new FJsonObject());
        RootJsonObject->SetObjectField(JsonField_RootProperty, NewJsonObject);
        RootPropertyJson = &NewJsonObject;
    }
    
    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!(*RootPropertyJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        /* フィールドが存在しない場合、新しいオブジェクトを作成 */
        TSharedPtr<FJsonObject> NewJsonObject = MakeShareable(new FJsonObject());
        (*RootPropertyJson)->SetObjectField(ObjectKey, NewJsonObject);
        ObjectJson = &NewJsonObject;
    }

    (*ObjectJson)->SetStringField(PropertyName, PropertyValue);

    UE_LOG(LogGDM, Verbose, TEXT("AddPropertyToJson: Added property '%s' with value '%s' to '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
}

void UGDMPropertyJsonSystemComponent::RemovePropertyFromJson(const FString& ObjectKey, const FString& PropertyName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJson: ObjectKey is empty."));
        return;
    }

    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJson: PropertyName is empty."));
        return;
    }

    const TSharedPtr<FJsonObject>* RootPropertyJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootProperty, RootPropertyJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJson: Property '%s' not found"), *JsonField_RootProperty);
        return;
    }
    
    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if ((*RootPropertyJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        if ((*ObjectJson)->HasField(PropertyName))
        {
            (*ObjectJson)->RemoveField(PropertyName);
            UE_LOG(LogGDM, Verbose, TEXT("RemovePropertyFromJson: Removed property '%s' from '%s'."), *PropertyName, *ObjectKey);
        }
        else
        {
            UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJson: Property '%s' not found in '%s'."), *PropertyName, *ObjectKey);
        }
    }
    else
    {
        UE_LOG(LogGDM, Warning, TEXT("RemovePropertyFromJson: ObjectKey '%s' not found."), *ObjectKey);
    }
}

bool UGDMPropertyJsonSystemComponent::ApplyJsonToObjectProperty(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty ObjectKey is empty."));
        return false;
    }

    if (!IsValid(TargetObject))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty TargetObject is nullptr."));
        return false;
    }
    
    if (PropertyName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty PropertyName is empty."));
        return false;
    }

    const TSharedPtr<FJsonObject>* RootPropertyJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootProperty, RootPropertyJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty Property '%s' not found"), *JsonField_RootProperty);
        return false;
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!(*RootPropertyJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty ObjectKey '%s' not found in JSON."), *ObjectKey);
        return false;
    }

    FString PropertyValue;
    if (!(*ObjectJson)->TryGetStringField(PropertyName, PropertyValue))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty Property '%s' not found in JSON for object '%s'."), *PropertyName, *ObjectKey);
        return false;
    }

    const FProperty* Property = TargetObject->GetClass()->FindPropertyByName(*PropertyName);
    if (Property == nullptr)
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty Property '%s' not found in target object '%s'."), *PropertyName, *TargetObject->GetName());
        return false;
    }

    void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    if (!Property->ImportText_Direct(*PropertyValue, PropertyValuePtr, nullptr, PPF_None))
    {
        UE_LOG(LogGDM, Warning, TEXT("ApplyJsonToObjectProperty Failed to set property '%s' with value '%s' for object '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
        return false;
    }

    UE_LOG(LogGDM, Verbose, TEXT("ApplyJsonToObjectProperty Successfully set property '%s' with value '%s' for object '%s'."), *PropertyName, *PropertyValue, *ObjectKey);
    return true;
}

void UGDMPropertyJsonSystemComponent::AddFunctionToJson(const FString& ObjectKey, UObject* TargetObject, const FString& FunctionName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddFunctionToJson: ObjectKey is empty."));
        return;
    }

    if (!IsValid(TargetObject))
    {
        UE_LOG(LogGDM, Warning, TEXT("AddFunctionToJson: TargetObject is null."));
        return;
    }

    if (FunctionName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("AddFunctionToJson: FunctionName is empty."));
        return;
    }
    
    const UFunction* Function = TargetObject->GetClass()->FindFunctionByName(*FunctionName);
    if (!IsValid(Function))
    {
        UE_LOG(LogGDM, Warning, TEXT("AddFunctionToJson: Function '%s' not found in object '%s'."), *FunctionName, *TargetObject->GetName());
        return;
    }
    
    const TSharedPtr<FJsonObject>* RootFunctionJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootFunction, RootFunctionJson))
    {
        /* フィールドが存在しない場合、新しいオブジェクトを作成 */
        TSharedPtr<FJsonObject> NewJsonObject = MakeShareable(new FJsonObject());
        RootJsonObject->SetObjectField(JsonField_RootFunction, NewJsonObject);
        RootFunctionJson = &NewJsonObject;
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!(*RootFunctionJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        /* フィールドが存在しない場合、新しいオブジェクトを作成 */
        TSharedPtr<FJsonObject> NewJsonObject = MakeShareable(new FJsonObject());
        (*RootFunctionJson)->SetObjectField(ObjectKey, NewJsonObject);
        ObjectJson = &NewJsonObject;
    }
    
    (*ObjectJson)->SetBoolField(FunctionName, true);
    
    UE_LOG(LogGDM, Verbose, TEXT("AddFunctionToJson: Added function '%s' with to '%s'."), *FunctionName, *ObjectKey);
}

void UGDMPropertyJsonSystemComponent::RemoveFunctionFromJson(const FString& ObjectKey, const FString& FunctionName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemoveFunctionFromJson: ObjectKey is empty."));
        return;
    }

    if (FunctionName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("RemoveFunctionFromJson: FunctionName is empty."));
        return;
    }

    const TSharedPtr<FJsonObject>* RootFunctionJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootFunction, RootFunctionJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("RemoveFunctionFromJson: Function '%s' not found"), *JsonField_RootFunction);
        return;
    }
    
    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if ((*RootFunctionJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        if ((*ObjectJson)->HasField(FunctionName))
        {
            (*ObjectJson)->RemoveField(FunctionName);
            UE_LOG(LogGDM, Verbose, TEXT("RemoveFunctionFromJson: Removed function '%s' from '%s'."), *FunctionName, *ObjectKey);
        }
        else
        {
            UE_LOG(LogGDM, Warning, TEXT("RemoveFunctionFromJson: Function '%s' not found in '%s'."), *FunctionName, *ObjectKey);
        }
    }
    else
    {
        UE_LOG(LogGDM, Warning, TEXT("RemoveFunctionFromJson: ObjectKey '%s' not found."), *ObjectKey);
    }
}

bool UGDMPropertyJsonSystemComponent::HaveFunctionInJson(const FString& ObjectKey, UObject* TargetObject, const FString& FunctionName) const
{
    if (ObjectKey.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson ObjectKey is empty."));
        return false;
    }

    if (!IsValid(TargetObject))
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson TargetObject is nullptr."));
        return false;
    }
    
    if (FunctionName.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson FunctionName is empty."));
        return false;
    }

    const UFunction* Function = TargetObject->GetClass()->FindFunctionByName(*FunctionName);
    if (!IsValid(Function))
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson Function '%s' not found in target object '%s'."), *FunctionName, *TargetObject->GetName());
        return false;
    }
    
    const TSharedPtr<FJsonObject>* RootFunctionJson = nullptr;
    if (!RootJsonObject->TryGetObjectField(JsonField_RootFunction, RootFunctionJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson Function '%s' not found"), *JsonField_RootFunction);
        return false;
    }

    const TSharedPtr<FJsonObject>* ObjectJson = nullptr;
    if (!(*RootFunctionJson)->TryGetObjectField(ObjectKey, ObjectJson))
    {
        UE_LOG(LogGDM, Warning, TEXT("HaveFunctionInJson ObjectKey '%s' not found in JSON."), *ObjectKey);
        return false;
    }

    return (*ObjectJson)->HasField(FunctionName);
}

void UGDMPropertyJsonSystemComponent::AddFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey)
{
    if (HasFavoriteEntry(DefinitionName, FavoriteSaveKey))
    {
        UE_LOG(LogGDM, Log, TEXT("AddFavoriteEntry: already exists (%s, %s)"), *DefinitionName, *FavoriteSaveKey);
        return;
    }
    
    TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
    Entry->SetStringField(JsonField_FavoriteDefinitionName, DefinitionName);
    Entry->SetStringField(JsonField_FavoriteSaveKey, FavoriteSaveKey);

    TArray<TSharedPtr<FJsonValue>> Array = RootJsonObject->HasTypedField<EJson::Array>(JsonField_RootFavorite)
        ? RootJsonObject->GetArrayField(JsonField_RootFavorite)
        : TArray<TSharedPtr<FJsonValue>>();

    Array.Add(MakeShared<FJsonValueObject>(Entry));

    RootJsonObject->SetArrayField(JsonField_RootFavorite, Array);

    UE_LOG(LogGDM, Verbose, TEXT("AddFavoriteEntry: DefinitionName %s, FavoriteSaveKey %s "), *DefinitionName, *FavoriteSaveKey);
}

bool UGDMPropertyJsonSystemComponent::RemoveFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey)
{
    if (!RootJsonObject->HasTypedField<EJson::Array>(JsonField_RootFavorite))
    {
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> Array = RootJsonObject->GetArrayField(JsonField_RootFavorite);
    const int32 OriginalCount = Array.Num();

    Array.RemoveAll([&](const TSharedPtr<FJsonValue>& Value)
    {
        const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
        if (Value->TryGetObject(ObjPtr))
        {
            return (*ObjPtr)->GetStringField(JsonField_FavoriteDefinitionName) == DefinitionName &&
                   (*ObjPtr)->GetStringField(JsonField_FavoriteSaveKey) == FavoriteSaveKey;
        }
        return false;
    });

    if (Array.Num() != OriginalCount)
    {
        /* 減ったら再セット */
        RootJsonObject->SetArrayField(JsonField_RootFavorite, Array);
        
        UE_LOG(LogGDM, Verbose, TEXT("RemoveFavoriteEntry: DefinitionName %s, FavoriteSaveKey %s  '%d'->'%d'"), *DefinitionName, *FavoriteSaveKey, OriginalCount, Array.Num());
        return true;
    }

    return false;
}

bool UGDMPropertyJsonSystemComponent::HasFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey) const
{
    if (!RootJsonObject->HasTypedField<EJson::Array>(JsonField_RootFavorite))
    {
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>> Array = RootJsonObject->GetArrayField(JsonField_RootFavorite);
    for (const TSharedPtr<FJsonValue>& Value : Array)
    {
        const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
        if (Value->TryGetObject(ObjPtr))
        {
            if ((*ObjPtr)->GetStringField(JsonField_FavoriteDefinitionName) == DefinitionName &&
                (*ObjPtr)->GetStringField(JsonField_FavoriteSaveKey) == FavoriteSaveKey)
            {
                return true;
            }
        }
    }

    return false;
}

TArray<FGDMFavoriteEntry> UGDMPropertyJsonSystemComponent::GetAllFavoriteEntries() const
{
    TArray<FGDMFavoriteEntry> OutEntries;

    if (!RootJsonObject->HasTypedField<EJson::Array>(JsonField_RootFavorite))
    {
        return OutEntries;
    }

    const TArray<TSharedPtr<FJsonValue>> JsonArray = RootJsonObject->GetArrayField(JsonField_RootFavorite);
    for (const TSharedPtr<FJsonValue>& Value : JsonArray)
    {
        const TSharedPtr<FJsonObject>* ObjPtr = nullptr;
        if (Value->TryGetObject(ObjPtr))
        {
            FGDMFavoriteEntry Entry;
            Entry.DefinitionName = (*ObjPtr)->GetStringField(JsonField_FavoriteDefinitionName);
            Entry.SaveKey = (*ObjPtr)->GetStringField(JsonField_FavoriteSaveKey);
            OutEntries.Add(Entry);
        }
    }

    return OutEntries;
}

void UGDMPropertyJsonSystemComponent::SetCustomStringArray(const FString& Key, const TArray<FString>& StringArray)
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("SetCustomStringArray: Key is empty."));
        return;
    }

    TSharedPtr<FJsonObject> RootCustomJson;
    if (RootJsonObject->HasTypedField<EJson::Object>(JsonField_RootCustom))
    {
        RootCustomJson = RootJsonObject->GetObjectField(JsonField_RootCustom);
    }
    else
    {
        RootCustomJson = MakeShared<FJsonObject>();
        RootJsonObject->SetObjectField(JsonField_RootCustom, RootCustomJson);
    }

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    for (const FString& Value : StringArray)
    {
        JsonArray.Add(MakeShared<FJsonValueString>(Value));
    }

    RootCustomJson->SetArrayField(Key, JsonArray);

    UE_LOG(LogGDM, Verbose, TEXT("SetCustomStringArray: Added array under key '%s'"), *Key);
}

void UGDMPropertyJsonSystemComponent::SetCustomString(const FString& Key, const FString& StringValue)
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("SetCustomString: Key is empty."));
        return;
    }

    TSharedPtr<FJsonObject> RootCustomJson;
    if (RootJsonObject->HasTypedField<EJson::Object>(JsonField_RootCustom))
    {
        RootCustomJson = RootJsonObject->GetObjectField(JsonField_RootCustom);
    }
    else
    {
        RootCustomJson = MakeShared<FJsonObject>();
        RootJsonObject->SetObjectField(JsonField_RootCustom, RootCustomJson);
    }

    RootCustomJson->SetStringField(Key, StringValue);

    UE_LOG(LogGDM, Verbose, TEXT("SetCustomString: Set '%s' to key '%s'."), *StringValue, *Key);
}

TArray<FString> UGDMPropertyJsonSystemComponent::GetCustomStringArray(const FString& Key) const
{
    TArray<FString> Result;
    
    if (!HasCustomString(Key))
    {
        return Result;
    }

    const TSharedPtr<FJsonObject> RootCustomJson = RootJsonObject->GetObjectField(JsonField_RootCustom);
    const TArray<TSharedPtr<FJsonValue>>* JsonArray = nullptr;

    if (RootCustomJson->TryGetArrayField(Key, JsonArray))
    {
        for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
        {
            if (Value->Type == EJson::String)
            {
                Result.Add(Value->AsString());
                UE_LOG(LogGDM, Verbose, TEXT("GetCustomStringArray: Add string value '%s' '%s'."), *Key, *Value->AsString());
            }
            else
            {
                UE_LOG(LogGDM, Warning, TEXT("GetCustomStringArray: Non-string value found in array for key '%s'."), *Key);
            }
        }
    }

    return Result;
}

FString UGDMPropertyJsonSystemComponent::GetCustomString(const FString& Key, const FString& DefaultValue) const
{
    if (HasCustomString(Key))
    {
        return RootJsonObject->GetObjectField(JsonField_RootCustom)->GetStringField(Key);
    }
    
    return DefaultValue;
}

bool UGDMPropertyJsonSystemComponent::HasCustomString(const FString& Key) const
{
    if (Key.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("HasCustomString: Key is empty."));
        return false;
    }
    
    if (!RootJsonObject->HasTypedField<EJson::Object>(JsonField_RootCustom))
    {
        return false;
    }

    return RootJsonObject->GetObjectField(JsonField_RootCustom)->HasField(Key);
}

FString UGDMPropertyJsonSystemComponent::GetJsonAsString() const
{
    FString JsonString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    if (FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), Writer))
    {
        UE_LOG(LogGDM, Verbose, TEXT("GetJsonAsString: %s"), *JsonString);
        return JsonString;
    }
    return TEXT("");
}

bool UGDMPropertyJsonSystemComponent::BuildJsonFromString(const FString& JsonString)
{
    if (JsonString.IsEmpty())
    {
        UE_LOG(LogGDM, Warning, TEXT("BuildJsonFromString: Input JSON string is empty."));
        return false;
    }

    TSharedPtr<FJsonObject> ParsedJsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, ParsedJsonObject) || !ParsedJsonObject.IsValid())
    {
        UE_LOG(LogGDM, Error, TEXT("BuildJsonFromString: Failed to parse JSON string."));
        return false;
    }

    RootJsonObject = ParsedJsonObject;
    
    UE_LOG(LogGDM, Verbose, TEXT("BuildJsonFromString: Successfully updated RootJsonObject."));
    return true;
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyBool(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyInt(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyFloat(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyByte(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyString(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyVector(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyVector2D(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}

void UGDMPropertyJsonSystemComponent::OnChangePropertyRotator(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old, const FString& PropertySaveKey)
{
    if (!PropertySaveKey.IsEmpty())
    {
        AddPropertyToJson(PropertySaveKey, PropertyOwnerObject, PropertyName.ToString());
    }
}
