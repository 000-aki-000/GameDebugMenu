/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuFunctions.generated.h"

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
	* DebugMenuの無効化する
	*
	* @memo TryCreateDebugMenuManagerが実行されない限り動作はしないが不要な検索処理などが発生してしまうので不要ならこれを使用する
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static void DisableGameDebugMenu();

	/**
	* 生成(同期ロードされるので注意)
	* マルチプレイの場合はPlayerControllerなど環境で呼ぶように
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool TryCreateDebugMenuManager(UObject* WorldContextObject, TSoftClassPtr<AGameDebugMenuManager> DebugMenuManagerClassPtr);

	/**
	* 削除
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool DestroyDebugMenuManager(UObject* WorldContextObject);

	/**
	* 取得(Managerの参照をもつことになるので使用する場合は注意)
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static AGameDebugMenuManager* GetGameDebugMenuManager(UObject* WorldContextObject);

	/**
	* 表示
	*
	* @return True: 処理の正常終了　False:マネージャがない
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool ShowDebugMenu(UObject* WorldContextObject);

	/**
	* 非表示
	*
	* @return True: 処理の正常終了　False:マネージャがない
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool HideDebugMenu(UObject* WorldContextObject);

	/**
	* (非)表示に切り替える
	*
	* @return True: 処理の正常終了　False:マネージャがない
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
	static UGameDebugMenuRootWidget* GetGameDebugMenuRootWidget(UObject* WorldContextObject);

	/**
	* DebugMenuに表示させるオブジェクトのプロパティを登録する
	*
	* @param TargetObject         - プロパティを所持してる対象のオブジェクト
	* @param PropertyUIConfigInfo - UIでの設定情報
	* @param PropertyName         - プロパティ名
	* @param DisplayCategoryName  - UI上のカテゴリ名,何も指定しなければ「Other」
	* @param DisplayPropertyName  - UI上のプロパティ名,何も指定しなければ「PropertyName」で表示
	* @param Description          - 説明文
	* @return True: 登録成功 False: 何らかの要因で失敗した（ログに出力されてます）
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", AdvancedDisplay = "4"))
	static bool RegisterGDMObjectProperty(UObject* TargetObject, const FGDMPropertyUIConfigInfo PropertyUIConfigInfo,FName PropertyName,const FText DisplayCategoryName,const FText DisplayPropertyName,const FText Description);

	/**
	* DebugMenuに表示させるオブジェクトの関数（カスタムイベント）を登録する
	*
	* @param TargetObject        - 関数（カスタムイベント）を所持してる対象のオブジェクト
	* @param FunctionName        - 関数（カスタムイベント）名
	* @param DisplayCategoryName - UI上のカテゴリ名,何も指定しなければ「Other」
	* @param DisplayFunctionName - UI上の関数（カスタムイベント）名,何も指定しなければ「FunctionName」
	* @param Description         - 説明文
	* @return True: 登録成功 False: 何らかの要因で失敗した（ログに出力されてます）
	* @memo ここで登録できるのは引数０の関数（カスタムイベント）のみ
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", AdvancedDisplay = "3"))
	static bool RegisterGDMObjectFunction(UObject* TargetObject,FName FunctionName,const FText DisplayCategoryName,const FText DisplayFunctionName,const FText Description);

	/**
	* 対象のオブジェクトが登録したプロパティ＆関数を解除する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static void UnregisterGDMObject(UObject* TargetObject);

	/**
	* 登録済みプロパティ情報を取得する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static UObject* GetGDMObjectProperty(UObject* WorldContextObject,const int32 Index, FText& OutDisplayCategoryName, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FName& OutEnumTypeName, FGDMPropertyUIConfigInfo& PropertyUIConfigInfo);

	/**
	* 登録済み関数（カスタムイベント）を取得する
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static UObject* GetGDMObjectFunction(UObject* WorldContextObject,const int32 Index, FText& OutDisplayCategoryName, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName);

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
	* 登録済みプロパティと関数の両方を考慮してソートしたカテゴリー名を取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static TArray<FText> GetSortedObjectPropertyAndFunctionCategoryNames(UObject* WorldContextObject);

	/**
	* 登録済みプロパティが使用できるか確認する
	*
	* @return True すべて問題なし　False 1つ以上使用できないものがあった
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool VerifyGDMNumObjectProperties(UObject* WorldContextObject);

	/**
	* 登録済み関数が使用できるか確認する
	*
	* @return True すべて問題なし　False 1つ以上使用できないものがあった
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool VerifyGDMNumObjectFunctions(UObject* WorldContextObject);

	/** 
	* コンソールコマンドの情報取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool GetGDMConsoleCommandNameByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandSingle& Out);

	/**
	* コンソールコマンド数
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static int32 GetGDMNumConsoleCommandNames();

	/** 
	* コンソールコマンドグループの情報取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool GetGDMConsoleCommandGroupByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandGroup& Out);

	/**
	* コンソールコマンドグループ数
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static int32 GetGDMNumConsoleCommandGroups();

	/** 
	* コンソールコマンドペアの情報取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool GetGDMConsoleCommandPairByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandPair& Out);

	/**
	* コンソールコマンドペア数
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static int32 GetGDMNumConsoleCommandPairs();

	/**
	* コンソールコマンドナンバーの情報取得
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static bool GetGDMConsoleCommandNumberByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandNumber& Out);

	/**
	* コンソールコマンドナンバー数
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static int32 GetGDMNumConsoleCommandNumbers();

	/**
	* コンソールコマンドのタイトル名を取得
	*/
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static TArray<FText> GetOrderConsoleCommandTitle();

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
	* DebugMenu操作オブジェクトの登録
	*
	* @param TargetInputObject - 対象のオブジェクト。未設定で呼び出した自身がセットされる
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (DefaultToSelf = "TargetInputObject", Keywords = "DebugMenu GDM"))
	static bool RegisterGDMInputObject(UObject* TargetInputObject);

	/**
	* DebugMenu操作オブジェクトの解除
	*
	* @param TargetInputObject - 対象のオブジェクト。未設定で呼び出した自身がセットされる
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (DefaultToSelf = "TargetInputObject", Keywords = "DebugMenu GDM"))
	static bool UnregisterGDMInputObject(UObject* TargetInputObject);

	/**
	* DebugReport用UIを表示する
	*
	* @memo 通常の開閉は最後に操作したMenuになるがこっちはなからずDebugReport用UIを表示されキャプチャもDebugMenuが含まれない
	*/
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static bool ShowDebugReport(UObject* WorldContextObject);

	/**
	* DebugReport用UIが対応するツールを取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static EGDMProjectManagementTool GetSelectedProjectManagementTool();

	/**
	* ビルド構成を文字列で取得
	*
	* @memo EBuildConfiguration(Engine\Source\Runtime\Core\Public\GenericPlatform\GenericPlatformMisc.h)参照
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetBuildConfigurationString();

	/**
	* エンジンのビルドバージョンを取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetBuildVersionString();

	/**
	* プロジェクトバージョンの取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static FString GetProjectVersionString();

	/**
	* ワールドのTimeDilationを取得
	*/
	UFUNCTION(BlueprintPure,Category = "GDM|Functions",meta = ( Keywords = "DebugMenu GDM",WorldContext = "WorldContextObject" ))
	static float GetWorldTimeDilation(UObject* WorldContextObject);

	/**
	* DebugMenuから切り替えできる言語を取得
	*/
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM"))
	static TArray<FString> GetGDMCultureList();
	
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject", DisplayName = "PrintLogGDM"))
	static void PrintLog(UObject* WorldContextObject, const FString& InString, float Duration = 0.2f);

	/**
	*
	*/
	UFUNCTION(BlueprintCallable, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", WorldContext = "WorldContextObject"))
	static void DynamicallyChangeBlockInput(UObject* WorldContextObject, bool bBlockFlag);

private:
	static void OnActorSpawnedClientWaitManager(AGameDebugMenuManager* SpawnDebugMenuManager);
	static void OnActorSpawnedServer(AActor* SpawnActor);
	static void ShowDebugConsoleCommand();
	static void HideDebugConsoleCommand();
	static void ToggleDebugConsoleCommand();
};
