/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMPropertyJsonSystemComponent.generated.h"

/**
 * DebugMenu全体で管理するJsonへの読み書きを管理するコンポーネント
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMPropertyJsonSystemComponent : public UActorComponent
{
	GENERATED_BODY()

private:
    TSharedPtr<FJsonObject> RootJsonObject;

public:
    UGDMPropertyJsonSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual void BeginPlay() override;

    /**
     * 対象のオブジェクトのプロパティをJsonに追加する
     * @param ObjectKey 
     * @param TargetObject 
     * @param PropertyName 
     */
    UFUNCTION(BlueprintCallable)
    void AddPropertyToJsonSingle(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const;

    /**
     * 対象のオブジェクトのプロパティをJsonに追加する（複数指定版）
     * @param ObjectKey 
     * @param TargetObject 
     * @param PropertyNames 
     */
    UFUNCTION(BlueprintCallable)
    void AddPropertyToJson(const FString& ObjectKey, UObject* TargetObject, const TArray<FString>& PropertyNames);

    /**
     * Jsonから指定プロパティを削除する
     * @param ObjectKey 
     * @param PropertyName 
     */
    UFUNCTION(BlueprintCallable)
    void RemovePropertyFromJsonSingle(const FString& ObjectKey, const FString& PropertyName) const;

    /**
     * Jsonから指定プロパティを削除する（複数指定版）
     * @param ObjectKey 
     * @param PropertyNames 
     */
    UFUNCTION(BlueprintCallable)
    void RemovePropertyToJson(const FString& ObjectKey, const TArray<FString>& PropertyNames);

    /**
     * Json内の指定プロパティ情報を対象のオブジェクトに反映する
     * @param ObjectKey 
     * @param TargetObject 
     * @param PropertyName 
     */
    UFUNCTION(BlueprintCallable)
    void ApplyJsonToObject(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName);

    /**
     * 配列の文字列をJsonにセットする
     * @param Key 
     * @param StringArray 
     */
    UFUNCTION(BlueprintCallable)
    void SetStringArrayToJson(const FString& Key, const TArray<FString>& StringArray);

    /**
     * 配列の文字列をJsonから取得する
     * @param Key 
     */
    UFUNCTION(BlueprintCallable)
    TArray<FString> GetStringArrayFromJson(const FString& Key) const;
    
    /**
     * 単一の文字列をJsonにセットする
     */
    UFUNCTION(BlueprintCallable)
    void SetSingleStringToJson(const FString& Key, const FString& StringValue);

    /**
     * 単一の文字列をJsonから取得
     */
    UFUNCTION(BlueprintCallable)
    FString GetSingleStringFromJson(const FString& Key, const FString& DefaultValue = TEXT("")) const;

    /**
     * Jsonを文字列で取得
     */
    UFUNCTION(BlueprintCallable,BlueprintPure=false)
    FString GetJsonAsString() const;

    /**
     * 文字列からJsonを構築する
     */
    bool BuildJsonFromString(const FString& JsonString);
    
private:

    UFUNCTION()
    void OnChangePropertyBool(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyInt(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyFloat(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyByte(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyString(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyVector(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyVector2D(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old, const FString& PropertySaveKey);

    UFUNCTION()
    void OnChangePropertyRotator(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old, const FString& PropertySaveKey);

};
