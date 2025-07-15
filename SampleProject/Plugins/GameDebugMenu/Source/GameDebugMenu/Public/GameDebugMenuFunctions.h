/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuTypes.h"
#include "GameDebugMenuFunctions.generated.h"

class UWidget;

UCLASS()
class GAMEDEBUGMENU_API UGameDebugMenuFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static TWeakObjectPtr<AGameDebugMenuManager> CurrentGameDebugMenuManager;
	static TArray<FGDMPendingObjectData> RegisterPendingProperties;
	static TArray<FGDMPendingObjectData> RegisterPendingFunctions;
	static FDelegateHandle ActorSpawnedDelegateHandle;
	static bool bDisableGameDebugMenu;
	
public:
	UGameDebugMenuFunctions(const FObjectInitializer& ObjectInitializer);

public:
	static void RegisterGameDebugMenuManagerInstance(AGameDebugMenuManager* RegisterManager);
	static void UnregisterGameDebugMenuManagerInstance(AGameDebugMenuManager* UnregisterManager);

public:
	/** 
	* 生成。ロードしてない場合は同期ロードされるので注意
	* @param PlayerController - メニューを操作するローカルのプレイヤーコントローラー
	* @param DebugMenuManagerClassName - 生成するマネージャークラス名(BP_GDM_Manager)
	* @return true: 正常終了 false: デバックメニューは使用できない
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool TryCreateDebugMenuManager(APlayerController* PlayerController, FString DebugMenuManagerClassName);
	
	/**
	* 削除
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool DestroyDebugMenuManager(APlayerController* PlayerController);

	/**
	* 取得(Managerの参照をもつことになるので使用する場合は注意)
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject", AdvancedDisplay = "bCheckInitialize"))
	static AGameDebugMenuManager* GetGameDebugMenuManager(const UObject* WorldContextObject, bool bCheckInitialize = true);

	/**
	* 表示
	* @return True: 処理の正常終了
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool ShowDebugMenu(UObject* WorldContextObject);

	/**
	* 非表示
	* @return True: 処理の正常終了
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool HideDebugMenu(UObject* WorldContextObject);

	/**
	* (非)表示に切り替える
	* @return True: 処理の正常終了
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool ToggleDebugMenu(UObject* WorldContextObject);

	/**
	* True: 表示中　False:非表示
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool IsShowingDebugMenu(UObject* WorldContextObject);

	/**
	* RootになるWidget取得(Widgetの参照をもつことになるので使用する場合は注意)
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static UGameDebugMenuRootWidget* GetGameDebugMenuRootWidget(const UObject* WorldContextObject);

	/**
	* DebugMenuに表示させるオブジェクトのプロパティを登録する
	* @param TargetObject - プロパティを所持してる対象のオブジェクト
	* @param PropertyUIConfigInfo - UIでの設定情報
	* @param PropertyName - プロパティ名
	* @param CategoryKey - カテゴリ
	* @param PropertySaveKey - プロパティデータ保存用の識別キー
	* @param DisplayPropertyName - UI上のプロパティ名,何も指定しなければ「PropertyName」で表示
	* @param Description - 説明文
	* @param DisplayPriority - リスト追加時の表示優先度（降順）
	* @return True: 登録成功 False: 何らかの要因で失敗した（ログに出力されてます）
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", AdvancedDisplay = "5"))
	static bool RegisterGDMObjectProperty(UObject* TargetObject, const FGDMPropertyUIConfigInfo PropertyUIConfigInfo, const FName PropertyName, const FGDMGameplayCategoryKey CategoryKey, const FString PropertySaveKey, const FText DisplayPropertyName,const FText Description,const int32 DisplayPriority);

	/**
	* DebugMenuに表示させるオブジェクトの関数（カスタムイベント）を登録する
	* @param TargetObject - 関数（カスタムイベント）を所持してる対象のオブジェクト
	* @param FunctionName - 関数（カスタムイベント）名
	* @param CategoryKey - カテゴリ
	* @param FunctionSaveKey - 関数保存用の識別キー
	* @param DisplayFunctionName - UI上の関数（カスタムイベント）名,何も指定しなければ「FunctionName」
	* @param Description - 説明文
	* @param DisplayPriority - リスト追加時の表示優先度（降順）
	* @return True: 登録成功 False: 何らかの要因で失敗した（ログに出力されてます）
	* @note ここで登録できるのは引数０の関数（カスタムイベント）のみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", AdvancedDisplay = "3"))
	static bool RegisterGDMObjectFunction(UObject* TargetObject, FName FunctionName, const FGDMGameplayCategoryKey CategoryKey, const FString FunctionSaveKey, const FText DisplayFunctionName,const FText Description, const int32 DisplayPriority);

	/**
	* 対象のオブジェクトが登録したプロパティ＆関数を解除する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static void UnregisterGDMObject(UObject* TargetObject);

	/**
	* 登録済みプロパティ情報を取得する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static UObject* GetGDMObjectProperty(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutPropertySaveKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& PropertyUIConfigInfo);

	/**
	* 登録済み関数（カスタムイベント）を取得する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static UObject* GetGDMObjectFunction(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutFunctionSaveKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName);

	/**
	* 登録済みプロパティ数を取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static int32 GetGDMNumObjectProperties(UObject* WorldContextObject);

	/**
	* 登録済み関数（カスタムイベント）数を取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static int32 GetGDMNumObjectFunctions(UObject* WorldContextObject);

	/**
	* 登録済みプロパティが使用できるか確認する
	* @return True すべて問題なし　False 1つ以上使用できないものがあった
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool VerifyGDMNumObjectProperties(UObject* WorldContextObject);

	/**
	* 登録済み関数が使用できるか確認する
	* @return True すべて問題なし　False 1つ以上使用できないものがあった
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool VerifyGDMNumObjectFunctions(UObject* WorldContextObject);

	/**
	* Gameplayメニューのカテゴリ名を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static TArray<FGDMMenuCategoryKey> GetGDMOrderGameplayCategoryTitle(UObject* WorldContextObject);

	/**
	* 無効なIndex
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static int32 GetGDMInvalidIndex();

	/**
	* オブジェクト名を取得する
	* 内部ではEditorならGetDisplayNameそれ以外でGetObjectNameを呼ぶ
	*/
	UFUNCTION(BlueprintPure,Category = "GDM|Functions",meta = ( Keywords = "DebugMenu GDM" ))
	static FString GetGDMObjectName(UObject* TargetObject);

	/**
	* デバッグメニュー操作用のInputComponentを登録する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static void RegisterInputComponentForGameDebugMenu(UObject* WorldContextObject, UInputComponent* InputComponent);
	
	/**
	* デバッグメニュー操作用のInputComponentを解除する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static void UnregisterInputComponentForGameDebugMenu(UObject* WorldContextObject, UInputComponent* InputComponent);
	
	/**
	* DebugReport用UIを表示する
	* @note 通常の開閉は最後に操作したMenuになるがこっちはなからずDebugReport用UIを表示されキャプチャもDebugMenuが含まれない
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool ShowDebugReport(UObject* WorldContextObject);

	/**
	* DebugReport用UIが対応するツールを取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static EGDMProjectManagementTool GetGDMSelectedProjectManagementTool();

	/**
	* ビルド構成を文字列で取得
	* @note EBuildConfiguration(Engine\Source\Runtime\Core\Public\GenericPlatform\GenericPlatformMisc.h)参照
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetGDMBuildConfigurationString();

	/**
	* エンジンのビルドバージョンを取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetGDMBuildVersionString();

	/**
	* プロジェクトバージョンの取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetGDMProjectVersionString();

	/**
	* デバッグメニュー用のログカテゴリを使用したPrintString
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject", DisplayName = "PrintLogScreenGDM"))
	static void PrintLogScreen(UObject* WorldContextObject, const FString& InString, float Duration = 0.2f, bool bPrintToLog = false);

	/**
	* デバッグメニュー用のStringTableから文字列を取得する
	* @param StringKey - DebugMenuのStringTableのキー
	* @param OutString - キー対応する文字列
	* @return true:成功。キーに合う文字列を取得　false:失敗。キーが存在しないかマネージャーが生成されてない
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool GetDebugMenuString(UObject* WorldContextObject, const FString StringKey, FString& OutString);

	/**
	* デバッグメニューから切り替えできる言語を取得(エンジン内の言語切り替え)
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static TArray<FString> GetGDMCultureList();
	
	/**
	* デバッグメニューの現在使用中の言語を取得
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static FName GetCurrentDebugMenuLanguage(UObject* WorldContextObject);
	
	/**
	* デバッグメニューで使用できる言語を取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static TArray<FName> GetDebugMenuLanguageKeys();
	
	/**
	* デバッグメニューで使用する共通の改行判定文字を取得
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetDebugMenuLineBreakString();
	
	/** 単体コマンドから識別子を構築 */
	UFUNCTION(BlueprintCallable, Category = "ConsoleCommand")
	static FString BuildConsoleCommandId_FromSingle(const FGDMConsoleCommandSingle& Command);

	/** グループコマンドから識別子を構築 */
	UFUNCTION(BlueprintCallable, Category = "ConsoleCommand")
	static FString BuildConsoleCommandId_FromGroup(const FGDMConsoleCommandGroup& Command);

	/** ペアコマンドから識別子を構築 */
	UFUNCTION(BlueprintCallable, Category = "ConsoleCommand")
	static FString BuildConsoleCommandId_FromPair(const FGDMConsoleCommandPair& Command);

	/** 数値指定コマンドから識別子を構築 */
	UFUNCTION(BlueprintCallable, Category = "ConsoleCommand")
	static FString BuildConsoleCommandId_FromNumber(const FGDMConsoleCommandNumber& Command);
	
public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (GDMMenuCategoryKey)", CompactNodeTitle = "=="), Category = "GDM|Functions")
	static bool EqualEqual_GDMMenuCategoryKey(const FGDMMenuCategoryKey& A, const FGDMMenuCategoryKey& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "NotEqual (GDMMenuCategoryKey)", CompactNodeTitle = "!="), Category = "GDM|Functions")
	static bool NotEqual_GDMMenuCategoryKey(const FGDMMenuCategoryKey& A, const FGDMMenuCategoryKey& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToByte (GDMMenuCategoryKey)", CompactNodeTitle = "->", BlueprintAutocast), Category = "GDM|Functions")
	static uint8 Conv_GDMMenuCategoryKeyToByte(const FGDMMenuCategoryKey& Key);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToGDMMenuCategoryKey (Byte)", CompactNodeTitle = "->", BlueprintAutocast), Category = "GDM|Functions")
	static FGDMMenuCategoryKey Conv_ByteToGDMMenuCategoryKey(const uint8& Key);

private:
	static void OnActorSpawnedClientWaitManager(AGameDebugMenuManager* SpawnDebugMenuManager);
	static void OnActorSpawnedServer(AActor* SpawnActor);
	static void ShowDebugConsoleCommand();
	static void HideDebugConsoleCommand();
	static void ToggleDebugConsoleCommand();
	static void ToggleInputSystemLog();
};
