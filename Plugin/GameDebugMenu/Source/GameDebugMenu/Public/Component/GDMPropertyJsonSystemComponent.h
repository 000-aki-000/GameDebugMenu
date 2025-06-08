/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuTypes.h"
#include "Components/ActorComponent.h"
#include "GDMPropertyJsonSystemComponent.generated.h"

/**
 * DebugMenu全体で管理するJsonへの読み書きを管理するコンポーネント
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class GAMEDEBUGMENU_API UGDMPropertyJsonSystemComponent : public UActorComponent
{
	GENERATED_BODY()

private:
    static const FString JsonField_RootProperty;
    static const FString JsonField_RootFunction;
    static const FString JsonField_RootCustom;
    static const FString JsonField_RootFavorite;
    static const FString JsonField_FavoriteDefinitionName;
    static const FString JsonField_FavoriteSaveKey;
    
    TSharedPtr<FJsonObject> RootJsonObject;

public:
    UGDMPropertyJsonSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual void BeginPlay() override;

public:
    /**
     * 対象のオブジェクトのプロパティをJsonに追加する
     */
    UFUNCTION(BlueprintCallable)
    void AddPropertyToJson(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const;
    
    /**
     * Jsonから指定プロパティを削除する
     */
    UFUNCTION(BlueprintCallable)
    void RemovePropertyFromJson(const FString& ObjectKey, const FString& PropertyName) const;

    /**
     * Json内のプロパティ情報を対象オブジェクトに反映する
     * @return true: 反映に成功 false: データがなかったか取得に失敗した
     */
    bool ApplyJsonToObjectProperty(const FString& ObjectKey, UObject* TargetObject, const FString& PropertyName) const;

    UFUNCTION(BlueprintCallable)
    void AddFunctionToJson(const FString& ObjectKey, UObject* TargetObject, const FString& FunctionName) const;

    UFUNCTION(BlueprintCallable)
    void RemoveFunctionFromJson(const FString& ObjectKey, const FString& FunctionName) const;

    bool HaveFunctionInJson(const FString& ObjectKey, UObject* TargetObject, const FString& FunctionName) const;

    /**
     * お気に入り情報を追加
     */
    UFUNCTION(BlueprintCallable)
    void AddFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey);

    /**
     * お気に入り情報を削除
     */
    UFUNCTION(BlueprintCallable)
    bool RemoveFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey);

    /**
     * お気に入り情報が存在するか？
     */
    UFUNCTION(BlueprintCallable)
    bool HasFavoriteEntry(const FString& DefinitionName, const FString& FavoriteSaveKey) const;

    /**
     * すべてのお気に入り情報を取得する
     */
    UFUNCTION(BlueprintCallable)
    TArray<FGDMFavoriteEntry> GetAllFavoriteEntries() const;
    
    /**
     * 配列の文字列をJsonにセットする
     */
    UFUNCTION(BlueprintCallable)
    void SetCustomStringArray(const FString& Key, const TArray<FString>& StringArray);

    /**
     * 単一の文字列をJsonにセットする
     */
    UFUNCTION(BlueprintCallable)
    void SetCustomString(const FString& Key, const FString& StringValue);

    /**
     * 配列の文字列をJsonから取得する
     * @param Key 
     */
    UFUNCTION(BlueprintCallable)
    TArray<FString> GetCustomStringArray(const FString& Key) const;
    
    /**
     * 単一の文字列をJsonから取得
     */
    UFUNCTION(BlueprintCallable)
    FString GetCustomString(const FString& Key, const FString& DefaultValue = TEXT("")) const;

    /**
     * Json内に指定されたキーで文字列が存在するかを確認する
     */
    UFUNCTION(BlueprintCallable)
    bool HasCustomString(const FString& Key) const;

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
