/**
* Copyright (c) 2022 akihiko moroi
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

	FTimerHandle WaitDebugMenuManagerTimerHandle;

protected:
	UPROPERTY()
	bool bActivateMenu;

public:
	/** イベント通知ディスパッチャー */
	UPROPERTY(BlueprintAssignable)
	FGDMWidgetDelegate OnSendWidgetEventDispatcher;

	UPROPERTY(BlueprintReadOnly, Category = "GDM", Meta = (ExposeOnSpawn = true))
	UGameDebugMenuWidget* ParentGameDebugMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GDM|Config")
	FString GameDebugMenuName;

public:
	virtual void AddToScreen(ULocalPlayer* LocalPlayer, int32 ZOrder) override;
	virtual void RemoveFromParent() override;

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
	*
	* @param bRequestDebugMenuManager - マネージャーから呼び出された場合trueになる。
	* @memo 呼ばれるのはActivateされてるときのみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event")
	void OnShowingMenu(bool bRequestDebugMenuManager);

	/**
	* UIを非表示にするときのイベント
	* 
	* @param bRequestDebugMenuManager - マネージャーから呼び出された場合trueになる。
	* @memo 呼ばれるのはActivateされてるときのみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|Event")
	void OnHidingMenu(bool bRequestDebugMenuManager);

	/**
	* アクティベート化をするときに呼ばれるイベント
	* 
	* @memo ここで表示したり、操作できる状態に移行する
	*/	
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnActivateDebugMenu(bool bAlwaysExecute);

	/**
	* ディアクティベート化をするときに呼ばれるイベント
	* 
	* @memo ここで非表示にしたり、メニューの終了処理を行う
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnDeactivateDebugMenu();

	/**
	* アクティベート状態であればTrue
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Event")
	virtual bool IsActivateDebugMenu();

	/**
	* UIのアクティベート化をする(表示、操作できる状態になる)
	* 
	* @param bAlwaysExecute - 基本Activate済みの場合無視するので必ず処理させる場合Trueにする
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void ActivateDebugMenu(bool bAlwaysExecute);

	/**
	* UIのディアクティベート化をする(非表示、操作不可状態になる)
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Event")
	virtual void DeactivateDebugMenu();

	/**
	* 選択状態かの判定（何をもって選択状態かの判断はWidget側に）
	*/
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category = "GDM")
	bool IsSelectedDebugMenu(UObject* TargetObject);

	UFUNCTION()
	virtual void OnChangeDebugMenuLanguage(const FName& NewLanguageKey, const FName& OldLanguageKey);

	/**
	* デバックメニューの使用言語が変更されたら呼ばれる
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void OnChangeDebugMenuLanguageBP(const FName& NewLanguageKey, const FName& OldLanguageKey);

	/**
	* 指定クラスの子供Widgetをすべて取得する
	* 
	* @param WidgetClass         - 対象のクラス
	* @param OutChildWidgets     - 取得できたWidget
	* @param bEndSearchAsYouFind - 最初に見つけた時点で終了する
	* @return true: １つ以上OutChildWidgetsがある　false: １つもWidgetがなかった
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM", meta = (DeterminesOutputType = "WidgetClass", DynamicOutputParam = "OutChildWidgets"))
	virtual bool GetWidgetChildrenOfClass(TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutChildWidgets, bool bEndSearchAsYouFind);
};
