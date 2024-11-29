/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameDebugMenuTypes.h"
#include "GameDebugMenuManager.generated.h"

class UGDMListenerComponent;
class UGameDebugMenuWidget;
class UGDMInputSystemComponent;
class UGDMScreenshotRequesterComponent;
class UGameDebugMenuWidgetDataAsset;
class UGameDebugMenuRootWidget;
class AGDMDebugCameraInput;
class UGDMPlayerControllerProxyComponent;
class AGDMDebugReportRequester;
class FGDMOutputDevice;

/**
* デバックメニュー管理マネージャー（生成はTryCreateDebugMenuManagerで）
*/
UCLASS(Blueprintable, NotBlueprintType, notplaceable)
class GAMEDEBUGMENU_API AGameDebugMenuManager : public AActor
{
	GENERATED_BODY()

	/** DebugMenuの入力操作を制御するコンポーネント */
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	UGDMInputSystemComponent* DebugMenuInputSystemComponent;

	/** DebugMenuで使用するスクリーンショットを制御するコンポーネント */
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	UGDMScreenshotRequesterComponent* ScreenshotRequesterComponent;

	/** DebugMenuの各イベント検知コンポーネント */
	UPROPERTY(Transient)
	UGDMListenerComponent* ListenerComponent;

protected:
	/** True：UI表示中 */
	bool bShowDebugMenu;

	/** メニュー開く前のポーズ判定 */
	bool bCachedGamePaused;

	/** メニュー開く前のマウスカーソル判定 */
	bool bCachedShowMouseCursor;

	/** スクリーンキャプチャ後レポート用UIを開く */
	bool bWaitToCaptureBeforeOpeningDebugReportMenu;

	/** メニュー開く前のナビゲーション情報 */
	TArray<TSharedRef<FNavigationConfig>> CachedNavigationConfigs;

	/** 登録済みプロパティ群 */
	TArray<TSharedPtr<FGDMObjectPropertyInfo>> ObjectProperties;

	/** 登録済み関数群 */
	TArray<TSharedPtr<FGDMObjectFunctionInfo>> ObjectFunctions;

	/** デバックメニュー用UIアセット */
	UPROPERTY(EditAnywhere, Category = "GDM|Config")
	UGameDebugMenuWidgetDataAsset* WidgetDataAsset;

	/** Viewport上に追加されてるメインWidget */
	UPROPERTY(Transient)
	UGameDebugMenuRootWidget* DebugMenuRootWidget;

	/** 生成した各メニューWidgetのインスタンス */
	UPROPERTY(Transient,BlueprintReadOnly, Category = "GDM")
	TMap<FString, UGameDebugMenuWidget* > DebugMenuInstances;

	/** True : デバックメニュー操作中ポーズする */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GDM|Config")
	bool bGamePause;

	/** デバックカメラ用のインプットアクター */
	UPROPERTY(EditAnywhere, Category = "GDM|Config")
	TSubclassOf<AGDMDebugCameraInput> DebugCameraInputClass;

	/** デバックカメラ用のインプットアクターのインスタンス */
	UPROPERTY(Transient)
	AGDMDebugCameraInput* DebugCameraInput;

	/** PlayerControllerに自動追加されるプロキシコンポーネント */
	UPROPERTY(EditAnywhere, Category = "GDM|Config")
	TSubclassOf<UGDMPlayerControllerProxyComponent> DebugMenuPCProxyComponentClass;

	/** DebugMenuのログデバイス（レポート送信用） */
	TSharedPtr<FGDMOutputDevice> OutputLog;

	UPROPERTY(Transient)
	TMap<FString, FString> DebugMenuStrings;

	UPROPERTY(BlueprintReadOnly, Category = "GDM")
	FName CurrentDebugMenuLanguage;

	UPROPERTY(Transient)
	bool bCurrentDebugMenuDirectStringKey;

	UPROPERTY(Transient)
	TArray<UGameDebugMenuWidget*> ViewportDebugMenuWidgets;

public:
	AGameDebugMenuManager(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void BeginPlay() override;

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void EnableInput(class APlayerController* PlayerController) override;
	virtual void DisableInput(class APlayerController* PlayerController) override;

public:
	UFUNCTION(BlueprintPure)
	UGDMInputSystemComponent* GetDebugMenuInputSystemComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMScreenshotRequesterComponent* GetScreenshotRequesterComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMListenerComponent* GetListenerComponent() const;

protected:
	/**
	* UIのルートWidgetの生成
	*/
	virtual void CreateDebugMenuRootWidget();

	/**
	* Navigation(無)有効（Menu開く前の状態にする）
	*/
	virtual void EnabledNavigationConfigs();
	virtual void DisabledNavigationConfigs();
	
	/**
	* DebugCamera用入力アクターの生成
	*/
	virtual void CreateDebugCameraInputClass();

	/**
	* マウスカーソルのオン・オフ（Menu開く前の状態にする）
	*/
	void EnableShowMouseCursorFlag(APlayerController* PlayerController);
	void RestoreShowMouseCursorFlag(APlayerController* PlayerController) const;

	/**
	* メニュー操作時のゲームポーズのオン・オフ（処理するかは「bGamePause」で判断）
	*/
	void TryEnableGamePause();
	void RestoreGamePause() const;

	/**
	* スクショ処理終了後呼ばれる
	*/
	UFUNCTION()
	virtual void OnScreenshotRequestProcessed();

	virtual void SyncLoadDebugMenuStringTables(FName TargetDebugMenuLanguage);

public:
	/**
	* コンソールコマンドの実行
	*/
	virtual void ExecuteConsoleCommand(const FString& Command, APlayerController* PC);

	/**
	* DebugMenuの表示する
	*/
	virtual bool ShowDebugMenu(bool bWaitToCaptureBeforeOpeningMenuFlag = false);

	/**
	* DebugMenuを非表示にする
	*/
	virtual void HideDebugMenu();

	/**
	* True:表示中 False:非表示
	*/
	virtual bool IsShowingDebugMenu();

	/**
	* メインとなるWidget取得
	*/
	virtual UGameDebugMenuRootWidget* GetDebugMenuRootWidget();

	/**
	* 各Debug画面となるWidgetクラスのキーリストを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	int32 GetAllDebugMenuKeys(TArray<FString>& OutKeys);

	/**
	* キーと紐付いた各Debug画面となるWidgetクラスを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	TSubclassOf<UGameDebugMenuWidget> GetDebugMenuWidgetClass(const FString& Key);

	/**
	* 生成済みのDebug画面となるWidgetを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	bool GetDebugMenuWidgetInstances(TArray<UGameDebugMenuWidget*>& OutInstances);

	/** 
	* ゲーム内のログ取得
	*/
	virtual void GetOutputLogStrings(TArray<FString>& OutLogs);
	virtual void GetOutputLogString(FString& OutLog, const FString& Separator);

	/** 
	* プロパティ＆関数の登録、取得、イベントの通知
	*/
	virtual EGDMPropertyType GetPropertyType(FProperty* TargetProperty);
	virtual bool RegisterObjectProperty(UObject* TargetObject, FName PropertyName, const FGDMGameplayCategoryKey& CategoryKey, const FText& DisplayPropertyName, const FText& Description, const FGDMPropertyUIConfigInfo& PropertyUIConfigInfo, const int32& DisplayPriority);
	virtual bool RegisterObjectFunction(UObject* TargetObject, FName FunctionName, const FGDMGameplayCategoryKey& CategoryKey, const FText& DisplayFunctionName, const FText& Description, const int32& DisplayPriority);
	virtual UObject* GetObjectProperty(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FName& OutEnumTypeName, FGDMPropertyUIConfigInfo& PropertyUIConfigInfo);
	virtual void RemoveObjectProperty(const int32 Index);
	virtual UObject* GetObjectFunction(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName);
	void RemoveObjectFunction(const int32 Index);
	int32 GetNumObjectProperties() const;
	int32 GetNumObjectFunctions() const;

	/**
	* ProxyComponentを対象PlayerControllerに追加する
	*/
	virtual void AddDebugMenuPCProxyComponent(APlayerController* PlayerController);

	/**
	* 入力イベントするオブジェクトの登録/解除
	*/
	virtual bool RegisterInputObject(UObject* TargetObject);
	virtual bool UnregisterInputObject(UObject* TargetObject);

	/**
	* DebugMenuの入力無視フラグの設定
	* (AControllerのSetIgnoreMoveInputと同様呼び出し回数を揃える必要がある)
	* 
	* @param bNewInput true : DebugMenuの入力が無視されるようになる　false : フラグを１つ解除する
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Input")
	virtual void SetIgnoreInput(bool bNewInput);

	/**
	* DebugMenuの入力無視状態フラグをリセットする
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Input")
	virtual void ResetIgnoreInput();

	/** 
	* DebugMenuの入力を無視する状態になっているか？
	*
	* @return True : 無視状態 False : 入力可能
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Input")
	virtual bool IsInputIgnored() const;

	/**
	* DebugMenuの言語を変更する
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	virtual void ChangeDebugMenuLanguage(FName LanguageKey, bool bForcedUpdate);

	/**
	* DebugMenu用のStringTableから文字列を取得する
	*/
	UFUNCTION(BlueprintPure, Category = "GDM")
	virtual bool GetDebugMenuString(const FString& StringKey, FString& OutString);

	UFUNCTION(BlueprintCallable, Category = "GDM")
	virtual TArray<FName> GetDebugMenuLanguageKeys();
	
	UFUNCTION(BlueprintCallable, Category = "GDM")
	TArray<UGameDebugMenuWidget*> GetViewportDebugMenuWidgets();

protected:
	virtual void CallExecuteConsoleCommandDispatcher(const FString& Command);
	virtual void CallShowDispatcher();
	virtual void CallHideDispatcher();
	virtual void CallRegisterInputSystemEventDispatcher(UObject* TargetObject);
	virtual void CallUnregisterInputSystemEventDispatcher(UObject* TargetObject);

	UFUNCTION()
	virtual void OnWidgetAdded(UWidget* AddWidget, ULocalPlayer* Player);
	
	UFUNCTION()
	virtual void OnWidgetRemoved(UWidget* RemoveWidget);

public:
	virtual void CallExecuteProcessEventDispatcher(const FName& FunctionName, UObject* TargetObject);
	virtual void CallChangePropertyBoolDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old);
	virtual void CallChangePropertyIntDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old);
	virtual void CallChangePropertyFloatDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old);
	virtual void CallChangePropertyByteDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old);
	virtual void CallChangePropertyStringDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old);
	virtual void CallChangePropertyVectorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old);
	virtual void CallChangePropertyVector2DDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old);
	virtual void CallChangePropertyRotatorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old);
	virtual void CallChangeActiveInputObjectDispatcher(UObject* NewTargetObject, UObject* OldTargetObject);
	virtual void CallChangeDebugMenuLanguageDispatcher(const FName& NewLanguageKey, const FName& OldLanguageKey);
	virtual void CallStartScreenshotRequestDispatcher();
	virtual void CallScreenshotRequestProcessedDispatcher();
};
