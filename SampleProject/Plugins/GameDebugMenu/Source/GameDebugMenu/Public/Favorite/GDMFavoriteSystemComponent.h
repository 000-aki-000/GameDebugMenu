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
#include "GDMFavoriteSystemComponent.generated.h"

class AGameDebugMenuManager;
class UGDMFavoriteItemDefinition;
class UGameDebugMenuManagerAsset;
class UUserWidget;
class UGDMPropertyJsonSystemComponent;

/**
 * メニューのお気に入りシステム制御コンポーネント
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMFavoriteSystemComponent : public UActorComponent
{
	GENERATED_BODY()

	/** お気に入り情報 */
	UPROPERTY()
	TArray<TObjectPtr<UGDMFavoriteItemDefinition>> CachedFavoriteDefinitions;
	
public:
	UGDMFavoriteSystemComponent();

protected:
	UGDMPropertyJsonSystemComponent* GetPropertyJsonSystemComponent() const;
	
public:
	virtual void Initialize(const UGameDebugMenuManagerAsset* InMenuAsset);

	/**
	 * お気に入りに追加/削除を行う
	 * @param TargetWidget - お気に入り対象のWidget
	 * @return true 追加/削除を行った　false お気に入り処理の対象外のWidgetだった
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool ToggleAddOrRemoveFavorite(UUserWidget* TargetWidget);

	/**
	 * お気に入りに追加する
	 */
	UFUNCTION(BlueprintCallable)
	virtual void AddFavorite(UUserWidget* TargetWidget);

	/**
	 * お気に入りから削除
	 */
	UFUNCTION(BlueprintCallable)
	virtual void RemoveFavorite(UUserWidget* TargetWidget);

	/**
	 * お気に入り処理ができるWidgetか？
	 */
	UFUNCTION(BlueprintPure)
	virtual bool CanFavorite(UUserWidget* TargetWidget);

	/**
	 * お気に入り中か？
	 */
	UFUNCTION(BlueprintPure)
	virtual bool IsFavorited(UUserWidget* TargetWidget);

	/**
	 * お気に入りから全削除をする
	 */
	UFUNCTION(BlueprintCallable)
	virtual void ClearAllFavorites();

	/**
	 * 指定のお気に入り情報が現在の情報と一致してるか？
	 * @return true 完全一致 
	 */
	UFUNCTION(BlueprintPure)
	bool IsFavoriteListEqualTo(const TArray<FGDMFavoriteEntry>& OtherList) const;

	/**
	 * お気に入り情報から関連するWidgetを生成し返す
	 */
	UFUNCTION(BlueprintCallable)
	virtual UUserWidget* CreateFavoriteWidgetFromEntry(const FGDMFavoriteEntry& Entry);
	
};
