/**
* Copyright (c) 2022 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameDebugMenuTypes.h"
#include "GameDebugMenuSettings.generated.h"

/**
* DebugMenu用設定クラス
*/
UCLASS(config=Game, defaultconfig)
class GAMEDEBUGMENU_API UGameDebugMenuSettings : public UObject
{
	GENERATED_BODY()

public:
	/** 起動時に実行されるコマンド郡 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TMap<FName,FGDMStartupConsoleCommandList> StartupConsoleCommands;

	/** 起動時に実行されるコマンド郡 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	FName StartupConsoleCommandKeyName;

	/** コンソールコマンド名 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandSingle> ConsoleCommandNames;
	
	/** コンソールコマンドグループ名（同時に複数コマンド実行） */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandGroup> ConsoleCommandGroups;

	/** コンソールコマンドペア名（トグルで２種のコマンド実行） */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandPair> ConsoleCommandPairs;

	/** コンソールコマンドナンバー（数値設定コマンド実行） */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandNumber> ConsoleCommandNumbers;

	/** エディターでのみ追加されるコンソールコマンド名 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandSingle> EditorOnlyConsoleCommandNames;

	/** エディターでのみ追加されるコンソールコマンドグループ名 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandGroup> EditorOnlyConsoleCommandGroups;

	/** エディターでのみ追加されるコンソールコマンドペア名 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandPair> EditorOnlyConsoleCommandPairs;

	/** エディターでのみ追加されるコンソールコマンドペア名 */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandNumber> EditorOnlyConsoleCommandNumbers;

	/** コンソールコマンドのカテゴリ名表示順(最大255) */
	UPROPERTY(EditAnywhere, config, Category = "ConsoleCommand")
	TArray<FGDMOrderMenuCategoryTitle> OrderConsoleCommandCategoryTitles;

	/** Gameplayメニューのカテゴリ名表示順(最大255) */
	UPROPERTY(EditAnywhere, config, Category = "Gameplay")
	TArray<FGDMOrderMenuCategoryTitle> OrderGameplayCategoryTitles;

	/** UI操作アクションマッピング */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	TMap<FName, FGDMActionMappingKey> ActionMappingKeys;

	/** UI操作Axisマッピング */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	TMap<FName, FGDMAxisMappingKey> AxisMappingKeys;

	/** ボタンが押されてからリピート処理をするまでの時間 */
	UPROPERTY(EditAnywhere, config, Category = "Input")	
	float ButtonRepeatDelay;

	/** リピート間隔 */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	float ButtonRepeatInterval;

	/** デバックメニューのUMG使用フォント */
	UPROPERTY(EditAnywhere, config, Category = "Font", meta = (AllowedClasses = "Font", DisplayName = "Font Family"))
	FSoftObjectPath FontName;

	/** バグレポート連携ツール */
	UPROPERTY(EditAnywhere, config, Category = "ReportSettings")
	EGDMProjectManagementTool ProjectManagementToolType;

	/** Redmine連携用設定情報 */
	UPROPERTY(EditAnywhere, config, Category = "ReportSettings")
	FGDMRedmineSettings	RedmineSettings;

	/** Trello連携用設定情報 */
	UPROPERTY(EditAnywhere, config, Category = "ReportSettings")
	FGDMTrelloSettings	TrelloSettings;

	/** Jira連携用設定情報 */
	UPROPERTY(EditAnywhere, config, Category = "ReportSettings")
	FGDMJiraSettings JiraSettings;

	/** バグレポート処理クラス */
	UPROPERTY(EditAnywhere, Category = "ReportSettings")
	TMap<EGDMProjectManagementTool, TSubclassOf<AGDMDebugReportRequester>> DebugReportRequesterClass;

	/** バグレポート用の画面キャプチャ処理を無効化する */
	UPROPERTY(EditAnywhere, Category = "ReportSettings")
	bool bDisableScreenCaptureProcessingWhenOpeningDebugMenu;

	/** DebugMenuから指定できるCultureのリスト */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	TArray<FString> CultureList;

	/** デバックメニュー用StringTable */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	TMap<FName, FGDMStringTableList> GameDebugMenuStringTables;

	/** デバックメニューの使用言語 */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	FName DefaultGameDebugMenuLanguage;

	/** デバックメニュー用の StringKey を指定してる箇所をそのまま表示する */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	bool bGameDebugMenuDirectStringKey;

public:
	UGameDebugMenuSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** ゲッター */
	static UGameDebugMenuSettings* Get()
	{
		return CastChecked<UGameDebugMenuSettings>(UGameDebugMenuSettings::StaticClass()->GetDefaultObject());
	}
	
	UObject* GetGDMFont();
	TArray<FText> GetIssueCategoryNameList();
	TArray<FText> GetPriorityNameList();
	TArray<FText> GetAssigneeNameList();
	int32 GetDefaultIssueCategoryIndex();
	int32 GetDefaultPriorityIndex();
	FString GetDebugMenuString(const FName& LanguageKey, const FString& StringKey);
	FString GetGameplayCategoryTitle(const int32& ArrayIndex);
	int32 GetGameplayCategoryIndex(const int32& ArrayIndex);
	TSubclassOf<AGDMDebugReportRequester>* GetDebugReportRequesterClass();

private:
	void SetupCategoryResets();
	void SetupCategorySlomo();
	void SetupCategoryCamera();
	void SetupCategoryProfiler();
	void SetupCategoryDisplay();
	void SetupCategoryViewMode();
	void SetupCategoryShowDebug();
	void SetupCategoryScalability();
	void SetupCategoryFreeze();
	void SetupCategoryDumpLogs();
	void SetupCategoryNetwork();
	void SetupCategorySound();
	void SetupCategoryAbilitySystem();
	void SetupCategoryOther();
	void SetupCategoryLogVerbosity();
	void SetupInputActions();

};
