/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"

#include "GameDebugMenuTypes.h"
#include "GameDebugMenuSettings.generated.h"

class UGameDebugMenuMasterAsset;
class AGameDebugMenuManager;
class IGDMDeveloperSettingProvider;
class AGDMDebugReportRequester;
class UGDMEnhancedInputComponent;

/**
* DebugMenu用設定クラス
*/
UCLASS(config=GameDebugMenu, DefaultConfig)
class GAMEDEBUGMENU_API UGameDebugMenuSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	friend class UGameDebugMenuDeveloperSettings;
	
public:
	/** マスターアセットのアセット名 */
	UPROPERTY(EditAnywhere, config, Category = "Meta")
	FString MasterAssetName;
	
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
	
	/** DebugMenuのWidgetの入力優先度 */
	UPROPERTY(EditAnywhere, config, Category = "Input")
	int32 WidgetInputActionPriority;

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

	/** バグレポート用の画面キャプチャ処理を無効化する */
	UPROPERTY(EditAnywhere, Category = "ReportSettings")
	bool bDisableScreenCaptureProcessingWhenOpeningDebugMenu;

	/** DebugMenuから指定できるCultureのリスト */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	TArray<FString> CultureList;

	/** デバックメニューの使用言語 */
	UPROPERTY(EditAnywhere, config, Category = "Localization")
	FName DefaultGameDebugMenuLanguage;

	/** true: SaveGameを使用する。 false: Jsonファイルを使用する */
	UPROPERTY(config, EditAnywhere, Category="Save")
	bool bUseSaveGame;

	/** JSON保存先のファイルパス（相対パス） */
	UPROPERTY(config, EditAnywhere, Category="Save")
	FString SaveFilePath;

	/** JSON保存先のファイル名 */
	UPROPERTY(config, EditAnywhere, Category="Save")
	FString SaveFileName;

	/** True: DebugMenuの保存機能を無効にする */
	UPROPERTY(config, EditAnywhere, Category="Save")
	bool bDisableSaveFile;

	/** True: コンソールコマンドは保存されない False: 実行したコンソールコマンドはMaxCommandHistoryNumまで保存する(超えたら古いものから上書き) */
	UPROPERTY(config, EditAnywhere, Category="Save")
	bool bDoesNotSaveConsoleCommand;

	/** 保存するコンソールコマンドの履歴件数 */
	UPROPERTY(config, EditAnywhere, Category="Save")
	int32 MaxCommandHistoryNum;

	/** ここに含まれる文字のコンソールコマンドは保存されない */
	UPROPERTY(config, EditAnywhere, Category="Save")
	TArray<FString> NoSaveConsoleCommands;
	
	/** DebugMenuでの改行文字 */
	UPROPERTY(EditAnywhere, config, Category = "Other")
	FString LineBreakString;

private:
	UPROPERTY(transient)
	mutable TObjectPtr<UGameDebugMenuMasterAsset> MasterAsset;

public:
	UGameDebugMenuSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FText GetSectionText() const override;
#endif

	TArray<FText> GetIssueCategoryNameList() const;
	TArray<FText> GetPriorityNameList() const;
	TArray<FText> GetAssigneeNameList() const;
	int32 GetDefaultIssueCategoryIndex() const;
	int32 GetDefaultPriorityIndex() const;
	FString GetGameplayCategoryTitle(const int32& ArrayIndex) const;
	int32 GetGameplayCategoryIndex(const int32& ArrayIndex) const;
	FString GetFullSavePath() const;

	const FGDMStringTableList* TryGetStringTableList(const FName& LanguageKey) const;
	TArray<FName> GetDebugMenuLanguageKeys() const;
	UClass* GetDebugMenuInputComponentClass() const;
	const TSubclassOf<AGDMDebugReportRequester>* GetDebugReportRequesterClass() const;
	FString GetDebugMenuString(const FName& LanguageKey, const FString& StringKey) const;
	UObject* GetDebugMenuFont() const;

	UGameDebugMenuMasterAsset* GetMasterAsset() const;
	
private:
	void SetupCategoryResets();
	void SetupCategorySlomo();
	void SetupCategoryCamera();
	void SetupCategoryProfiler();
	void SetupCategoryDisplay();
	void SetupCategoryShowDebug();
	void SetupCategoryViewMode();
	void SetupCategoryScalability();
	void SetupCategoryFreeze();
	void SetupCategoryDumpLogs();
	void SetupCategoryNetwork();
	void SetupCategorySound();
	void SetupCategoryAbilitySystem();
	void SetupCategoryOther();
	void SetupCategoryLogVerbosity();

};
