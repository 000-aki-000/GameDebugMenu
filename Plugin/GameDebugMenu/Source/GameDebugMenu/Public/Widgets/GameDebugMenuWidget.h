/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameDebugMenuTypes.h"
#include "GameDebugMenuWidget.generated.h"

class UInputAction;
class UGDMEnhancedInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMWidgetDelegate, UGameDebugMenuWidget*, TargetWidget, FName, EventName);

DECLARE_DYNAMIC_DELEGATE( FOnGameDebugMenuWidgetInputAction );

/**
 * DebugMenu用Widgetの基底クラス
 * ページを増やす場合はこれを継承させて作成
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEDEBUGMENU_API UGameDebugMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** イベント通知ディスパッチャー */
	UPROPERTY(BlueprintAssignable)
	FGDMWidgetDelegate OnSendWidgetEventDispatcher;
	
protected:
	bool bActivateMenu;
	TArray<uint32> InputHandles;

public:
	UGameDebugMenuWidget(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeInputComponent() override;

public:
	/**
	 * ローカルのコントローラーを取得（DebugCamera操作中でも取得できるもの）
	 */
	UFUNCTION(BlueprintPure, Category = "GDM")
	APlayerController* GetOriginalPlayerController() const;
	
	/**
	 * Widgetが所持するInputComponentを返す 
	 */
	UFUNCTION(BlueprintPure, Category = "GDM|Input")
	UGDMEnhancedInputComponent* GetMyInputComponent() const;
	
	/**
	* 指定のInputActionが入力されたときに呼び出される関数名を登録する
	* @param Action - 対象となる入力アクション
	* @param FunctionName - 呼び出される関数名
	* @param TriggerEvent - 入力イベント
	* @param FunctionObject - 関数名を持つオブジェクト。nullなら自分自身
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Input", meta = (AdvancedDisplay = "2"))
	bool RegisterDebugMenuWidgetInputFunction(const UInputAction* Action, const FName FunctionName, const ETriggerEvent TriggerEvent = ETriggerEvent::Triggered, UObject* FunctionObject = nullptr);

	/**
	* 指定のInputActionが入力されたときに呼び出されるイベントを登録する
	* @param Action - 対象となる入力アクション
	* @param Callback - 入力時に呼び出されるイベント
	* @param TriggerEvent - 入力イベント
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Input", meta = (AdvancedDisplay = "2"))
	bool RegisterDebugMenuWidgetInputEvent(const UInputAction* Action, FOnGameDebugMenuWidgetInputAction Callback, const ETriggerEvent TriggerEvent = ETriggerEvent::Triggered);

	/**
	 * 登録した入力イベントを解除する
	 */
	UFUNCTION(BlueprintCallable, Category = "GDM|Input")
	void UnregisterDebugMenuWidgetInputs();
	
	/**
	* イベント通知
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	void SendSelfEvent(FName EventName);

	/**
	* コンソールコマンドを実行する
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Command")
	void ExecuteGDMConsoleCommand(const FString Command, const EGDMConsoleCommandNetType CommandNetType);

	/**
	* UIを表示するときのイベント
	* @param bRequestDebugMenuManager - マネージャーから呼び出された場合trueになる。
	* @note 呼ばれるのはActivateされてるときのみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event", meta = (AdvancedDisplay ="bRequestDebugMenuManager"))
	void OnShowingMenu(bool bRequestDebugMenuManager);

	/**
	* UIを非表示にするときのイベント
	* @param bRequestDebugMenuManager - マネージャーから呼び出された場合trueになる。
	* @note 呼ばれるのはActivateされてるときのみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event", meta = (AdvancedDisplay ="bRequestDebugMenuManager"))
	void OnHidingMenu(bool bRequestDebugMenuManager);

	/**
	* アクティベート化をするときに呼ばれるイベント
	* @note ここで表示したり、操作できる状態に移行する
	*/	
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnActivateDebugMenu();

	/**
	* ディアクティベート化をするときに呼ばれるイベント
	* @note ここで非表示にしたり、メニューの終了処理を行う
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnDeactivateDebugMenu();

	/**
	 * 画面を最新の状態し直したいときに呼ばれるイベント
	 * @note UI上で参照するデータを取得し直すなどをここでする
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event")
	void OnRefreshDataAndDisplay();
	
	/**
	* アクティベート状態であればTrue
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Event")
	virtual bool IsActivateDebugMenu();

	/**
	* UIのアクティベート化をする(表示、操作できる状態になる)
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void ActivateDebugMenu();

	/**
	* UIのディアクティベート化をする(非表示、操作不可状態になる)
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void DeactivateDebugMenu();

	UFUNCTION()
	virtual void OnChangeDebugMenuLanguage(const FName& NewLanguageKey, const FName& OldLanguageKey);

	/**
	* デバックメニューの使用言語が変更されたら呼ばれる
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Language")
	void OnChangeDebugMenuLanguageBP(const FName& NewLanguageKey, const FName& OldLanguageKey);

	/**
	* 指定クラスの子供Widgetをすべて取得する
	* @param WidgetClass         - 対象のクラス
	* @param OutChildWidgets     - 取得できたWidget
	* @param bEndSearchAsYouFind - 最初に見つけた時点で終了する
	* @return true: １つ以上OutChildWidgetsがある　false: １つもWidgetがなかった
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM", meta = (DeterminesOutputType = "WidgetClass", DynamicOutputParam = "OutChildWidgets"))
	virtual bool GetWidgetChildrenOfClass(TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutChildWidgets, bool bEndSearchAsYouFind);
};
