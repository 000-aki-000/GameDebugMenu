/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GDMFavoriteItemDefinition.generated.h"

class AGameDebugMenuManager;
class UUserWidget;

/**
 * お気に入り登録できるWidget情報を定義
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class GAMEDEBUGMENU_API UGDMFavoriteItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	/* お気に入り登録されるWidgetクラス */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> FavoriteWidgetClass = nullptr;

	TWeakObjectPtr<AGameDebugMenuManager> OwnerManager = nullptr;
	
public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "GDM")
	AGameDebugMenuManager* GetOwnerManager() const;
	
	/**
	 * この定義がサポートするWidgetか？
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GDM")
	bool IsSupportedWidget(UUserWidget* Widget) const;
	virtual bool IsSupportedWidget_Implementation(UUserWidget* Widget) const PURE_VIRTUAL(, return false;);
	
	/**
	 * エクスポート処理
	 * @return Widgetを復元するために必要な識別キーを返す 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GDM")
	FString ExportToFavoriteKey(UUserWidget* TargetWidget);
	virtual FString ExportToFavoriteKey_Implementation(UUserWidget* TargetWidget) PURE_VIRTUAL(, return FString(););
	
	/**
	 * インポート処理＋Widget生成
	 * @param FavoriteSaveKey - Widgetを復元するために必要な識別キー
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GDM")
	UUserWidget* CreateWidgetFromFavoriteData(const FString& FavoriteSaveKey);
	virtual UUserWidget* CreateWidgetFromFavoriteData_Implementation(const FString& FavoriteSaveKey) PURE_VIRTUAL(, return nullptr;);
};
