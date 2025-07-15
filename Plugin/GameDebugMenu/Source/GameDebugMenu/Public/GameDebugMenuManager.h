/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameDebugMenuTypes.h"
#include "GameDebugMenuManager.generated.h"

class UGDMConsoleCommandSetAsset;
class UGDMFavoriteSystemComponent;
class UGDMConsoleCommandValueProviderComponent;
class UGDMLocalizeStringComponent;
class UGDMSaveSystemComponent;
class UGDMPropertyJsonSystemComponent;
class UGDMListenerComponent;
class UGameDebugMenuWidget;
class UGDMInputSystemComponent;
class UGDMScreenshotRequesterComponent;
class UGameDebugMenuManagerAsset;
class UGameDebugMenuRootWidget;
class UGDMPlayerControllerProxyComponent;
class AGDMDebugReportRequester;
class FGDMOutputDevice;

/**
* デバックメニュー管理マネージャー（生成はTryCreateDebugMenuManagerで）
*/
UCLASS(Blueprintable, BlueprintType, notplaceable)
class GAMEDEBUGMENU_API AGameDebugMenuManager : public AActor
{
	GENERATED_BODY()

protected:
	/** DebugMenuの入力操作を制御するコンポーネント */
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMInputSystemComponent> DebugMenuInputSystemComponent;

	/** DebugMenuで使用するスクリーンショットを制御するコンポーネント */
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMScreenshotRequesterComponent> ScreenshotRequesterComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMPropertyJsonSystemComponent> PropertyJsonSystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMSaveSystemComponent> SaveSystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMFavoriteSystemComponent> FavoriteSystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMConsoleCommandValueProviderComponent> ConsoleCommandValueProviderComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMLocalizeStringComponent> LocalizeStringComponent;
	
	/** DebugMenuの各イベント検知コンポーネント */
	UPROPERTY(VisibleAnywhere, Category = "GDM", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGDMListenerComponent> ListenerComponent;

	FTimerHandle InitializeManagerHandle;
	bool bInitializedManager = false;

	/** デバックメニュー用UIアセット */
	UPROPERTY(EditAnywhere, Category = "GDM")
	TObjectPtr<UGameDebugMenuManagerAsset> MenuAsset;

	/** コンソールコマンド設定アセット */
	UPROPERTY(EditAnywhere, Category = "GDM")
	TObjectPtr<UGDMConsoleCommandSetAsset> ConsoleCommandSetAsset;
	
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
	
	/** Viewport上に追加されてるメインWidget */
	UPROPERTY(Transient)
	TObjectPtr<UGameDebugMenuRootWidget> DebugMenuRootWidget;

	/** 生成した各メニューWidgetのインスタンス */
	UPROPERTY(Transient,BlueprintReadOnly, Category = "GDM")
	TMap<FString, TObjectPtr<UGameDebugMenuWidget>> DebugMenuInstances;

	/** Viewport上に追加されてるメニューWidgetのインスタンス */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameDebugMenuWidget>> ViewportDebugMenuWidgets;

	/** DebugMenuのログデバイス（レポート送信用） */
	TSharedPtr<FGDMOutputDevice> OutputLog;

public:
	AGameDebugMenuManager(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION(BlueprintPure)
	UGDMInputSystemComponent* GetDebugMenuInputSystemComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMScreenshotRequesterComponent* GetScreenshotRequesterComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMPropertyJsonSystemComponent* GetPropertyJsonSystemComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMSaveSystemComponent* GetSaveSystemComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMFavoriteSystemComponent* GetFavoriteSystemComponent() const;

	UFUNCTION(BlueprintPure)
	UGDMConsoleCommandValueProviderComponent* GetConsoleCommandValueProviderComponent() const;
	
	UFUNCTION(BlueprintPure)
	UGDMLocalizeStringComponent* GetLocalizeStringComponent() const;
	
	UFUNCTION(BlueprintPure)
	UGDMListenerComponent* GetListenerComponent() const;

	UFUNCTION(BlueprintPure, Category = "GDM")
	bool IsInitializedManager() const;

protected:
	/**
	* マネージャーの初期化
	*/
	virtual void OnInitializeManager();

	UFUNCTION(BlueprintImplementableEvent, Category="GDM", meta=(DisplayName="OnInitializeManager", ScriptName="OnInitializeManager"))
	void OnInitializeManagerBP();
	
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
	* オーナーのPlayerControllerを取得
	*/
	UFUNCTION(BlueprintPure, Category = "GDM")
	APlayerController* GetOwnerPlayerController() const;
	
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
	* 各Debug画面となるWidgetのインスタンスからキーを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	FString GetDebugMenuWidgetKey(const UGameDebugMenuWidget* Widget);
	
	/**
	* 生成済みのDebug画面となるWidgetを取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM")
	bool GetDebugMenuWidgetInstances(TArray<UGameDebugMenuWidget*>& OutInstances);

	/** 
	* ゲーム内のログ取得
	*/
	virtual void GetOutputLogString(FString& OutLog, const FString& Separator);

	/** 
	* ゲーム中実行したコンソールコマンドの履歴を取得
	*/
	virtual void GetOutputCommandHistoryString(TArray<FString>& OutCommandHistory);

	UFUNCTION(BlueprintCallable, Category = "GDM")
	virtual void ClearCommandHistory();

	virtual EGDMPropertyType GetPropertyType(const FProperty* TargetProperty) const;
	virtual bool RegisterObjectProperty(UObject* TargetObject, const FName PropertyName, const FGDMGameplayCategoryKey& CategoryKey, const FString& PropertySaveKey, const FText& DisplayPropertyName, const FText& Description, const FGDMPropertyUIConfigInfo& PropertyUIConfigInfo, const int32& DisplayPriority);
	virtual bool RegisterObjectFunction(UObject* TargetObject, const FName FunctionName, const FGDMGameplayCategoryKey& CategoryKey, const FString& FunctionSaveKey, const FText& DisplayFunctionName, const FText& Description, const int32& DisplayPriority);
	virtual UObject* GetObjectProperty(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutPropertySaveKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& OutPropertyUIConfigInfo);
	virtual void RemoveObjectProperty(const int32 Index);
	virtual UObject* GetObjectFunction(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutFunctionSaveKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName);
	void RemoveObjectFunction(const int32 Index);
	int32 GetNumObjectProperties() const;
	int32 GetNumObjectFunctions() const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	UObject* TryGetObjectProperty(const FString& InPropertySaveKey, const FString& InPropertyName, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayPropertyName, FText& OutDescription, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& OutPropertyUIConfigInfo) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	UObject* TryGetObjectFunction(const FString& InFunctionSaveKey, const FString& InFunctionName, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayFunctionName, FText& OutDescription) const;

	/**
	* ProxyComponentを対象PlayerControllerに追加する
	*/
	virtual void AddDebugMenuPCProxyComponent(APlayerController* PlayerController);
	
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
	 * Viewport上にあるすべてのGameDebugMenuWidgetを取得する
	 */
	UFUNCTION(BlueprintCallable, Category = "GDM")
	TArray<UGameDebugMenuWidget*> GetViewportDebugMenuWidgets();

	UFUNCTION(BlueprintCallable, Category = "GDM")
	void ChangeConsoleCommandSetAsset(UGDMConsoleCommandSetAsset* NewCommandSetAsset); 

	UFUNCTION(BlueprintPure, Category = "GDM")
	UGDMConsoleCommandSetAsset* GetConsoleCommandSetAsset() const;
	
protected:
	virtual void CallExecuteConsoleCommandDispatcher(const FString& Command);
	virtual void CallShowDispatcher();
	virtual void CallHideDispatcher();

	UFUNCTION()
	virtual void OnWidgetAdded(UWidget* AddWidget, ULocalPlayer* Player);
	
	UFUNCTION()
	virtual void OnWidgetRemoved(UWidget* RemoveWidget);

public:
	virtual void CallExecuteProcessEventDispatcher(const FName& FunctionName, UObject* TargetObject);
	virtual void CallChangePropertyBoolDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyIntDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyFloatDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyByteDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyStringDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyVectorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyVector2DDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old, const FString& PropertySaveKey);
	virtual void CallChangePropertyRotatorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old, const FString& PropertySaveKey);
	virtual void CallChangeDebugMenuLanguageDispatcher(const FName& NewLanguageKey, const FName& OldLanguageKey);
	virtual void CallStartScreenshotRequestDispatcher();
	virtual void CallScreenshotRequestProcessedDispatcher();
	virtual void CallLoadedDebugMenuDispatcher();
	virtual void CallSavedDebugMenuDispatcher();
	virtual void CallDeletedDebugMenuDispatcher();
	
};
