/**
* Copyright (c) 2025 akihiko moroi
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

public:
    /**
     * 対象のオブジェクトのプロパティをJsonに追加する
     * @param ObjectKey 
     * @param TargetObject 
     * @param PropertyName 
     */
    UFUNCTION(BlueprintCallable)
    void AddPropertyToJson(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const;
    
    /**
     * Jsonから指定プロパティを削除する
     * @param ObjectKey 
     * @param PropertyName 
     */
    UFUNCTION(BlueprintCallable)
    void RemovePropertyFromJson(const FString& ObjectKey, const FString& PropertyName) const;

    /**
     * Json内の指定プロパティ情報を対象のオブジェクトに反映する
     * @param ObjectKey 
     * @param TargetObject 
     * @param PropertyName
     * @return true: 反映に成功 false: データがなかったか取得に失敗した
     */
    UFUNCTION(BlueprintCallable)
    bool ApplyJsonToObject(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName);

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
     * Json内に指定されたキーで文字列が存在するかを確認する
     */
    UFUNCTION(BlueprintCallable)
    bool HasStringInJson(const FString& Key) const;

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
