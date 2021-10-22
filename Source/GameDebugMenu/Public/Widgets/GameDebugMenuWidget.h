/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameDebugMenuTypes.h"
#include "Input/GDMInputInterface.h"
#include "GameDebugMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMWidgetDelegate, UGameDebugMenuWidget*, TargetWidget, FName, EventName);

/**
 * DebugMenu用Widgetの基底クラス
 * ページを増やす場合はこれを継承させて作成
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEDEBUGMENU_API UGameDebugMenuWidget : public UUserWidget
	, public IGDMInputInterface
{
	GENERATED_BODY()

public:
	/** イベント通知ディスパッチャー */
	UPROPERTY(BlueprintAssignable)
	FGDMWidgetDelegate OnSendWidgetEventDispatcher;

	UPROPERTY()
	bool bActivateMenu;

	UPROPERTY(BlueprintReadOnly, Category = "GDM", Meta = (ExposeOnSpawn = true))
	UGameDebugMenuWidget* ParentGameDebugMenuWidget;

public:
	/**
	* イベント通知
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	void SendSelfEvent(FName EventName);

	/**
	* コマンドを実行する
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Command")
	void ExecuteGDMConsoleCommand(const FString Command, EGDMConsoleCommandNetType CommandNetType);

	/**
	* UIを表示するときのイベント
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event")
	void OnShowingMenu();

	/**
	* UIを非表示にするときのイベント
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event")
	void OnClosingMenu();

	/**
	* アクティベート化をするときに呼ばれるイベント
	*/	
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnActivateDebugMenu(bool bAlwaysExecute);

	/**
	* ディアクティベート化をするときに呼ばれるイベント
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnDeactivateDebugMenu();

	/**
	* アクティベート状態であればTrue
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Event")
	virtual bool IsActivateDebugMenu();

	/**
	* UIのアクティベート化をする
	* @param 基本Activate済みの場合無視するので必ず処理させる場合Trueにする
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void ActivateDebugMenu(bool bAlwaysExecute);

	/**
	* UIのディアクティベート化をする
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void DeactivateDebugMenu();

	/**
	* 選択状態かの判定（何をもって選択状態かの判断はWidget側に）
	*/
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category = "GDM")
	bool IsSelectedDebugMenu(UObject* TargetObject);
};
