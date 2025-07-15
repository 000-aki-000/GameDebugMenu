/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuTypes.h"
#include "Engine/DataAsset.h"
#include "GameDebugMenuManagerAsset.generated.h"

class AGDMDebugCameraInput;
class UGDMPlayerControllerProxyComponent;
class UGameDebugMenuRootWidget;
class UGameDebugMenuWidget;
class UGDMFavoriteItemDefinition;

/**
 * デバッグメニューの構成をまとめたデータアセット
 * このアセットは、GameDebugMenuManager によって参照され、
 * 表示するデバッグメニューUIの構成・入力設定などメニュー全体の挙動を制御する設定を一元的に保持
 */
UCLASS(Const)
class GAMEDEBUGMENU_API UGameDebugMenuManagerAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** メインとなるデバックメニューのWidgetクラス */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<UGameDebugMenuRootWidget> DebugMenuRootWidgetClass;

	/** 各メニューのWidget郡(Key=メニュー識別名, Value=メニュークラス) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TMap<FString, TSubclassOf<UGameDebugMenuWidget> > DebugMenuClasses;

	/** 各メニューのUI登録順（メニュー識別名を指定する） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Widget")
	TArray<FString> DebugMenuRegistrationOrder;

	/** メインとなるデバックメニューのWidgetのZオーダー値 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	int32 RootWidgetZOrder;

	/** DebugMenuManagerが作られたときに追加される InputMappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenCreateManager;
	 
	/** DebugMenuを表示するときに追加される InputMappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenDebugMenuIsShow;

	/** デバックカメラ用のインプットアクター */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSubclassOf<AGDMDebugCameraInput> DebugCameraInputClass;

	/** お気に入りデータの定義 */
	UPROPERTY(EditAnywhere, Instanced, category = "Favorite")
	TArray<TObjectPtr<UGDMFavoriteItemDefinition>> FavoriteItemDefinitions;
	
	/** PlayerControllerに自動追加されるコンポーネント */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Other")
	TSubclassOf<UGDMPlayerControllerProxyComponent> DebugMenuPCProxyComponentClass;
	
	/** True : デバックメニュー操作中ポーズする */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Other")
	bool bGamePause;
	
public:
	UGameDebugMenuManagerAsset();
};
