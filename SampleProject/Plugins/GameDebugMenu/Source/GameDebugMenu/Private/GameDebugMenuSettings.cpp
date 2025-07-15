/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuSettings.h"
#include "Performance/EnginePerformanceTargets.h"
#include <Internationalization/StringTableCore.h>

#include "GameDebugMenuTypes.h"
#include "Components/Widget.h"
#include "Data/GameDebugMenuMasterAsset.h"
#include "Engine/AssetManager.h"
#include "Input/GDMEnhancedInputComponent.h"
#include "Reports/GDMRequesterJira.h"
#include "Reports/GDMRequesterRedmine.h"
#include "Reports/GDMRequesterTrello.h"

UGameDebugMenuSettings::UGameDebugMenuSettings()
{
	CategoryName = TEXT("Plugins");

	SetupCategoryResets();
	SetupCategorySlomo();
	SetupCategoryCamera();
	SetupCategoryProfiler();
	SetupCategoryDisplay();
	SetupCategoryShowDebug();
	SetupCategoryViewMode();
	SetupCategoryScalability();
	SetupCategoryFreeze();
	SetupCategoryDumpLogs();
	SetupCategoryNetwork();
	SetupCategorySound();
	SetupCategoryAbilitySystem();
	SetupCategoryOther();
	SetupCategoryLogVerbosity();

	MasterAssetName = TEXT("DA_GDM_Master");
	
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Resets"),0));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Slomo"),1));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Camera"),2));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Profiler"),3));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Display"),4));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Show Debug"),5));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("ViewMode"),6));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Scalability"),7));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Freeze"),8));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Dump Logs"),9));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Network"),10));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Sounds"),11));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Ability System"),12));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Other"),13));
	OrderConsoleCommandCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Log Verbosity"),14));

	OrderGameplayCategoryTitles.Add(FGDMOrderMenuCategoryTitle(TEXT("Other"),0));

	/* AGameDebugMenuManagerもデフォルトではInt最大値なのでそれより低くする
	 * 同じ、または大きくした場合、マネージャーで設定する入力はメニューが閉じられるまで反応しなくなるので注意 */
	WidgetInputActionPriority = TNumericLimits<int32>::Max() - 1;

	CultureList.Add(TEXT("ja"));
	CultureList.Add(TEXT("en"));
	
	DefaultGameDebugMenuLanguage = TEXT("Japanese");

	bDisableScreenCaptureProcessingWhenOpeningDebugMenu = true;

	bUseSaveGame = false;
	SaveFilePath = TEXT("Saved/DebugMenu");
	SaveFileName = TEXT("DebugMenuSaveData");
	bDisableSaveFile = false;
	bDoesNotSaveConsoleCommand = false;
	MaxCommandHistoryNum = 100;
	NoSaveConsoleCommands.Reset();
	NoSaveConsoleCommands.Add(TEXT("LevelEditor."));
	NoSaveConsoleCommands.Add(TEXT("ToggleDebugCamera"));
	NoSaveConsoleCommands.Add(TEXT("stat "));
	NoSaveConsoleCommands.Add(TEXT("LoadTimes."));
	NoSaveConsoleCommands.Add(TEXT("CsvProfile "));
	NoSaveConsoleCommands.Add(TEXT("Obj "));
	NoSaveConsoleCommands.Add(TEXT("Freeze"));
	LineBreakString = TEXT("\n");
	
	MasterAsset = nullptr;
}

#if WITH_EDITOR
void UGameDebugMenuSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	int32 Count = INDEX_NONE;
	for( auto& CategoryTitle : OrderConsoleCommandCategoryTitles )
	{
		CategoryTitle.PreviewTitle = GetDebugMenuString(DefaultGameDebugMenuLanguage, CategoryTitle.Title);
		if( CategoryTitle.Index > Count )
		{
			Count = CategoryTitle.Index;
		}
	}

	for( auto& CategoryTitle : OrderConsoleCommandCategoryTitles )
	{
		if( CategoryTitle.Index == INDEX_NONE )
		{
			++Count;
			CategoryTitle.Index = Count;
		}
	}

	Count = INDEX_NONE;
	for( auto& CategoryTitle : OrderGameplayCategoryTitles )
	{
		CategoryTitle.PreviewTitle = GetDebugMenuString(DefaultGameDebugMenuLanguage, CategoryTitle.Title);
		if( CategoryTitle.Index > Count )
		{
			Count = CategoryTitle.Index;
		}
	}

	for( auto& CategoryTitle : OrderGameplayCategoryTitles )
	{
		if( CategoryTitle.Index == INDEX_NONE )
		{
			++Count;
			CategoryTitle.Index = Count;
		}
	}
}

FText UGameDebugMenuSettings::GetSectionText() const
{
	return FText::FromString(TEXT("Game Debug Menu"));
}
#endif

TArray<FText> UGameDebugMenuSettings::GetIssueCategoryNameList() const
{
	switch (ProjectManagementToolType)
	{
		case EGDMProjectManagementTool::Trello:
		{
			return TrelloSettings.CardListNames;
		}
		case EGDMProjectManagementTool::Redmine:
		{
			return RedmineSettings.TrackerNameList;
		}
		case EGDMProjectManagementTool::Jira:
		{
			return JiraSettings.IssueTypeList;
		}
	}

	TArray<FText> Empty;
	return Empty;
}

TArray<FText> UGameDebugMenuSettings::GetPriorityNameList() const
{
	switch (ProjectManagementToolType)
	{
		case EGDMProjectManagementTool::Trello:
		{
			break;/* 特になし */
		}
		case EGDMProjectManagementTool::Redmine:
		{
			return RedmineSettings.PriorityNameList;
		}
		case EGDMProjectManagementTool::Jira:
		{
			return JiraSettings.PriorityNameList;
		}
	}

	TArray<FText> Empty;
	return Empty;
}

TArray<FText> UGameDebugMenuSettings::GetAssigneeNameList() const
{
	switch(ProjectManagementToolType)
	{
		case EGDMProjectManagementTool::Trello:
		{
			/* 未対応 */
			break;
		}
		case EGDMProjectManagementTool::Redmine:
		{
			/* 未対応 */
			break;
		}
		case EGDMProjectManagementTool::Jira:
		{
			TArray<FText> TextList;
			JiraSettings.AssigneeList.GenerateValueArray(TextList);
			return TextList;
		}
	}

	TArray<FText> Empty;
	return Empty;
}

int32 UGameDebugMenuSettings::GetDefaultIssueCategoryIndex() const
{
	switch (ProjectManagementToolType)
	{
		case EGDMProjectManagementTool::Trello:
		{
			break;/* 特になし */
		}
		case EGDMProjectManagementTool::Redmine:
		{
			return RedmineSettings.DefaultTrackerIndex;
		}
		case EGDMProjectManagementTool::Jira:
		{
			return JiraSettings.DefaultIssueTypeIndex;
		}
	}

	return 0;
}

int32 UGameDebugMenuSettings::GetDefaultPriorityIndex() const
{
	switch (ProjectManagementToolType)
	{
		case EGDMProjectManagementTool::Trello:
		{
			break;/* 特になし */
		}
		case EGDMProjectManagementTool::Redmine:
		{
			return RedmineSettings.DefaultPriorityIndex;
		}
		case EGDMProjectManagementTool::Jira:
		{
			return JiraSettings.DefaultPriorityIndex;
		}
	}

	return 0;
}

FString UGameDebugMenuSettings::GetGameplayCategoryTitle(const int32& ArrayIndex) const
{
	FString ReturnValue;
	if( OrderGameplayCategoryTitles.IsValidIndex(ArrayIndex) )
	{
		const FString& Title = OrderGameplayCategoryTitles[ArrayIndex].Title;
		ReturnValue = GetDebugMenuString(DefaultGameDebugMenuLanguage, Title);
		if( ReturnValue.IsEmpty() )
		{
			ReturnValue = Title;
		}
	}

	return ReturnValue;
}

int32 UGameDebugMenuSettings::GetGameplayCategoryIndex(const int32& ArrayIndex) const
{
	if( OrderGameplayCategoryTitles.IsValidIndex(ArrayIndex) )
	{
		return OrderGameplayCategoryTitles[ArrayIndex].Index;
	}

	return 0;
}

FString UGameDebugMenuSettings::GetFullSavePath() const
{
	return FPaths::ProjectDir().Append(SaveFilePath).Append(TEXT("/")).Append(SaveFileName).Append(TEXT(".json"));
}

const FGDMStringTableList* UGameDebugMenuSettings::TryGetStringTableList(const FName& LanguageKey) const
{
	if (const auto Master = GetMasterAsset())
	{
		return Master->GameDebugMenuStringTables.Find(LanguageKey);
	}
	
	return nullptr;
}

TArray<FName> UGameDebugMenuSettings::GetDebugMenuLanguageKeys() const
{
	TArray<FName> ReturnValues;

	if (const auto Master = GetMasterAsset())
	{
		Master->GameDebugMenuStringTables.GetKeys(ReturnValues);
	}
	
	return ReturnValues;
}

UClass* UGameDebugMenuSettings::GetDebugMenuInputComponentClass() const
{
	TSoftClassPtr<UGDMEnhancedInputComponent> Class = nullptr;

	if (const auto Master = GetMasterAsset())
	{
		Class = Master->DebugMenuInputComponentClass;	
	}
	
	ensureMsgf(Class.IsValid(), TEXT("Invalid DebugMenuInputComponentClass class in GameDebugMenuSettings. Manual reset required."));
	
	return Class.IsValid() ? Class.Get() : UGDMEnhancedInputComponent::StaticClass();
}

const TSubclassOf<AGDMDebugReportRequester>* UGameDebugMenuSettings::GetDebugReportRequesterClass() const
{
	if (const auto Master = GetMasterAsset())
	{
		return Master->DebugReportRequesterClass.Find(ProjectManagementToolType);	
	}

	return nullptr;
}


FString UGameDebugMenuSettings::GetDebugMenuString(const FName& LanguageKey, const FString& StringKey) const
{
	if( const FGDMStringTableList* StringTableList = TryGetStringTableList(LanguageKey) )
	{
		FString OutSourceString;

		for( auto& StrTablePtr : StringTableList->StringTables )
		{
			if( !StrTablePtr.ToSoftObjectPath().IsValid() || StrTablePtr.ToSoftObjectPath().IsNull() )
			{
				UE_LOG(LogGDM, Warning, TEXT("GetDebugMenuString: failed StringTable : LanguageKey->%s StringKey->%s"), *LanguageKey.ToString(), *StringKey);
				continue;
			}

			if(const UStringTable* StringTable = StrTablePtr.LoadSynchronous() )
			{
				const FStringTableConstPtr TableData = StringTable->GetStringTable();
				if (!TableData.IsValid() || !TableData->IsLoaded())
				{
					UE_LOG(LogGDM, Warning, TEXT("GetDebugMenuString: StringTable not loaded: %s"), *StringTable->GetName());
					continue;
				}

				if( TableData->GetSourceString(StringKey, OutSourceString) )
				{
					return OutSourceString;
				}
			}
		}		
	}
	
	return FString();
}

UObject* UGameDebugMenuSettings::GetDebugMenuFont() const
{
	if (const auto Master = GetMasterAsset())
	{
		if (UObject* FontObj = Master->FontName.ResolveObject())
		{
			return FontObj;
		}

		return Master->FontName.TryLoad();
	}

	const FSoftObjectPath EngineFontPath = UWidget::GetDefaultFontName();
	return EngineFontPath.TryLoad();
}

UGameDebugMenuMasterAsset* UGameDebugMenuSettings::GetMasterAsset() const
{
	const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!IsValid(AssetManager))
	{
		return nullptr;
	}

	if (MasterAssetName.IsEmpty())
	{
		UE_LOG(LogGDM, Warning, TEXT("MasterAssetName is empty."));
		return nullptr;
	}

	const FPrimaryAssetId MasterAssetId(UGameDebugMenuMasterAsset::GetPrimaryType(), FName(*MasterAssetName));

	if (IsValid(MasterAsset))
	{
		const FPrimaryAssetId CachedId = MasterAsset->GetPrimaryAssetId();
		if (CachedId == MasterAssetId)
		{
			return MasterAsset.Get();
		}
	}
	
	UObject* Asset = AssetManager->GetPrimaryAssetObject(MasterAssetId);
	if (!IsValid(Asset))
	{
		const FSoftObjectPath AssetPath = AssetManager->GetPrimaryAssetPath(MasterAssetId);
		Asset = AssetPath.TryLoad();
	}

	MasterAsset = Cast<UGameDebugMenuMasterAsset>(Asset);
	
	return MasterAsset;
}

void UGameDebugMenuSettings::SetupCategoryResets()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex		= 0; /* Resets */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Stat None"));
	Single.Description        = FText::FromString(TEXT("Statをクリアする"));
	Single.ConsoleCommandName = TEXT("stat None");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Grouped"));
	Single.Description        = FText::FromString(TEXT("stat Slow を無効"));
	Single.ConsoleCommandName = TEXT("stat Grouped");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Load Times Report Reset"));
	Single.Description        = FText::FromString(TEXT("loadtimes.dumpreportの出力情報をリセットする"));
	Single.ConsoleCommandName = TEXT("LoadTimes.reset");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 0; /* Resets */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Toggle All Screen Messages"));
	Pair.Description              = FText::FromString(TEXT("トグルで画面ログの(非)表示を切り替える"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("DisableAllScreenMessages");
	Pair.SecondConsoleCommandName = TEXT("EnableAllScreenMessages");
	ConsoleCommandPairs.Add(Pair);
}

void UGameDebugMenuSettings::SetupCategorySlomo()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 1; /* Slomo */

	Single.Title              = FText::FromString(TEXT("Reset"));
	Single.Description        = FText::FromString(TEXT("Slomoを1にリセットする"));
	Single.ConsoleCommandName = TEXT("Slomo 1");
	Single.CommandNetType     = EGDMConsoleCommandNetType::ServerAll;
	ConsoleCommandNames.Add(Single);
	
	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 1; /* Slomo */
	Number.PreConsoleCommandName = TEXT("");
	Number.PostConsoleCommandName = TEXT(" ");

	Number.Title                            = FText::FromString(TEXT("Change Slomo"));
	Number.Description                      = FText::FromString(TEXT("Slomoを変更し反映させる"));
	Number.ConsoleCommandName               = TEXT("Slomo");
	Number.ConsoleVariableName				= TEXT("Slomo");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 10.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 0.1f;
	Number.UIConfigInfo.MaxChangeAmount     = 0.2f;
	Number.DefaultValue                     = 1.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	ConsoleCommandNumbers.Add(Number);
}

void UGameDebugMenuSettings::SetupCategoryCamera()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 2; /* Camera */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Debug Camera"));
	Single.Description        = FText::FromString(TEXT("ゲーム中のカメラを離れ、デバッグ用の別カメラに切り替える\n同時に注視点のアセットの情報を画面に表示します"));
	Single.ClickedEvent       = EGDMConsoleCommandClickedEvent::MenuClose;
	Single.ConsoleCommandName = TEXT("ToggleDebugCamera");
	ConsoleCommandNames.Add(Single);

	Single.CommandNetType     = EGDMConsoleCommandNetType::ServerAll;
	Single.Title              = FText::FromString(TEXT("Teleport Player controlled pawn"));
	Single.Description        = FText::FromString(TEXT("プレイヤー操作ポーンをカメラの注視点にテレポートさせる"));
	Single.ClickedEvent		  = EGDMConsoleCommandClickedEvent::Non;
	Single.ConsoleCommandName = TEXT("Teleport");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 2; /* Camera */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Debug Camera Trace Complex"));
	Pair.Description              = FText::FromString(TEXT(""));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("g.DebugCameraTraceComplex 0");
	Pair.SecondConsoleCommandName = TEXT("g.DebugCameraTraceComplex 1");
	ConsoleCommandPairs.Add(Pair);
}

void UGameDebugMenuSettings::SetupCategoryProfiler()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 3; /* Profiler */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("FPS"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat fps");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Unit"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat Unit");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Unit Graph"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat UnitGraph");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Unit Max"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat UnitMax");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Unit Time"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat UnitTime");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Raw"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat Raw");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandGroup Group;
	Group.CategoryIndex = 3; /* Profiler */
	Group.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	/* EditorOnly */

	Group.Title       = FText::FromString(TEXT("ProfileGPU ShowUI"));
	Group.Description = FText::FromString(TEXT("GPUの処理負荷を階層的に出力（エディター用UI表示版）\nエディターでのみ有効"));
	Group.ConsoleCommandNames.Add(TEXT("r.ProfileGPU.ShowUI 1"));
	Group.ConsoleCommandNames.Add(TEXT("ProfileGPU"));
	EditorOnlyConsoleCommandGroups.Add(Group);
	Group.ConsoleCommandNames.Empty();

	/* EditorOnly */

	Group.Title       = FText::FromString(TEXT("ProfileGPU"));
	Group.Description = FText::FromString(TEXT("GPUの処理負荷を階層的に出力"));
	Group.ConsoleCommandNames.Add(TEXT("r.ProfileGPU.ShowUI 0"));
	Group.ConsoleCommandNames.Add(TEXT("ProfileGPU"));
	ConsoleCommandGroups.Add(Group);
	Group.ConsoleCommandNames.Empty();

	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 3; /* Profiler */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Toggle RHISetGPUCaptureOptions"));
	Pair.Description              = FText::FromString(TEXT("BasePass内部の各DrawCall毎の処理負荷を出力する\n(マテリアル名などが見られるようになったり) ※RHIThreadがOffになり、DrawThread負荷が増加する点に注意"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("r.RHISetGPUCaptureOptions 1");
	Pair.SecondConsoleCommandName = TEXT("r.RHISetGPUCaptureOptions 0");
	ConsoleCommandPairs.Add(Pair);

	Pair.Title                    = FText::FromString(TEXT("Toggle Profile"));
	Pair.Description              = FText::FromString(TEXT("startfile / stopfile をトグルで実行。プロファイル結果をファイルにue4stats形式で出力する"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::MenuClose;
	Pair.FirstConsoleCommandName  = TEXT("stat startfile");
	Pair.SecondConsoleCommandName = TEXT("stat stopfile");
	ConsoleCommandPairs.Add(Pair);

	Pair.Title                    = FText::FromString(TEXT("Toggle CSV Profiler"));
	Pair.Description              = FText::FromString(TEXT("CsvProfile Start / Stop のトグル"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::MenuClose;
	Pair.FirstConsoleCommandName  = TEXT("CsvProfile Start");
	Pair.SecondConsoleCommandName = TEXT("CsvProfile Stop");
	ConsoleCommandPairs.Add(Pair);
}

void UGameDebugMenuSettings::SetupCategoryDisplay()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 4; /* Display */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Game"));
	Single.Description        = FText::FromString(TEXT("各種 Gameplay のティックの所要時間に関するフィードバックです"));
	Single.ConsoleCommandName = TEXT("stat Game");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Engine"));
	Single.Description        = FText::FromString(TEXT("フレーム時間やレンダリング中のトライアングル数のカウンタなど\n一般的なレンダリング統計情報を表示します"));
	Single.ConsoleCommandName = TEXT("stat Engine");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Init Views"));
	Single.Description        = FText::FromString(TEXT("カリングの所要時間やプリミティブ数などの情報を表示する"));
	Single.ConsoleCommandName = TEXT("stat InitViews");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Scene Rendering"));
	Single.Description        = FText::FromString(TEXT("一般的なレンダリング統計を示す"));
	Single.ConsoleCommandName = TEXT("stat SceneRendering");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("GPU"));
	Single.Description        = FText::FromString(TEXT("ProfileGPUのリアルタイム版。GPUのフレーム単位の統計を表示する"));
	Single.ConsoleCommandName = TEXT("stat GPU");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("RHI"));
	Single.Description        = FText::FromString(TEXT("全体のDrawCall数などを確認できる"));
	Single.ConsoleCommandName = TEXT("stat RHI");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Particles"));
	Single.Description        = FText::FromString(TEXT("パーティクルの統計情報。今どのくらい出ていて、どのくらい処理がかかっているのかを表示する"));
	Single.ConsoleCommandName = TEXT("stat GPUParticles");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Levels"));
	Single.Description        = FText::FromString(TEXT("現在読み込まれているLevelの表示。読み込みにかかった時間も表示される"));
	Single.ConsoleCommandName = TEXT("stat Levels");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LightRendering"));
	Single.Description        = FText::FromString(TEXT("ライティングとシャドウのレンダリングにかかる時間に関する情報を表示"));
	Single.ConsoleCommandName = TEXT("stat LightRendering");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShadowRendering"));
	Single.Description        = FText::FromString(TEXT("stat LightRendering で示される実際のシャドウ レンダリング時間とは別に\nシャドウの計算にかかっている時間を表示"));
	Single.ConsoleCommandName = TEXT("stat ShadowRendering");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Streaming"));
	Single.Description        = FText::FromString(TEXT("テクスチャのストリーミング処理で使用しているメモリ量、シーン内に存在するストリーミング中のテクスチャ数など\nストリーミング中のアセットの各種統計情報を表示"));
	Single.ConsoleCommandName = TEXT("stat Streaming");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Streaming Details"));
	Single.Description        = FText::FromString(TEXT("一般的なテクスチャ ストリーミングをさらに特定のグループ\n (ライトマップ、静的 テクスチャ、動的テクスチャ) に分類するなど\nストリーミングに関するより詳しい統計情報を表示"));
	Single.ConsoleCommandName = TEXT("stat StreamingDetails");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Memory"));
	Single.Description        = FText::FromString(TEXT("メモリ使用量の統計を表示する"));
	Single.ConsoleCommandName = TEXT("stat Memory");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Animation"));
	Single.Description        = FText::FromString(TEXT("SkeletalMeshなどのティックごとの計算時間を表示する"));
	Single.ConsoleCommandName = TEXT("stat Anim");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Slate"));
	Single.Description        = FText::FromString(TEXT("Slateで使用するTickなどを表示する"));
	Single.ConsoleCommandName = TEXT("stat Slate");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Hitches"));
	Single.Description        = FText::FromString(TEXT("ヒッチを検知してログに出力する"));
	Single.ConsoleCommandName = TEXT("stat hitches");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Character"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat character");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("GameplayTags"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat GameplayTags");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Collision"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("stat Collision");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Component"));
	Single.Description        = FText::FromString(TEXT("コンポーネントのパフォーマンス情報(Transform更新など)リストを表示する"));
	Single.ConsoleCommandName = TEXT("stat component");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LLM"));
	Single.Description        = FText::FromString(TEXT("Low Level Memory Tracker(LLM)はメモリの使用状況を追跡するツールを使用し\nTag付けされた使用中のメモリを表示\n(LLM有効にはならないので事前に設定すること)"));
	Single.ConsoleCommandName = TEXT("stat llm");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LLMPlatform"));
	Single.Description        = FText::FromString(TEXT("OSの情報を含むメモリ情報を表示"));
	Single.ConsoleCommandName = TEXT("stat LLMPlatform");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("UObjects"));
	Single.Description        = FText::FromString(TEXT("ゲーム内の UObjects のパフォーマンスに関する統計情報を表示"));
	Single.ConsoleCommandName = TEXT("stat UObjects");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandGroup Group;
	Group.CategoryIndex = 4; /* Display */
	Group.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Group.Title       = FText::FromString(TEXT("AI Info"));
	Group.Description = FText::FromString(TEXT("AIに必要な情報(BehaviorTree,EQS)をまとめて表示する"));
	Group.ConsoleCommandNames.Add(TEXT("stat AI"));
	Group.ConsoleCommandNames.Add(TEXT("stat AIBehaviorTree"));
	Group.ConsoleCommandNames.Add(TEXT("stat AI_EQS"));
	Group.ConsoleCommandNames.Add(TEXT("stat Navigation"));
	ConsoleCommandGroups.Add(Group);
	Group.ConsoleCommandNames.Empty();


	Group.Title       = FText::FromString(TEXT("All Memory Info"));
	Group.Description = FText::FromString(TEXT("使用量、他アロケーター情報などを一括表示する"));
	Group.ConsoleCommandNames.Add(TEXT("stat MemoryPlatform"));
	Group.ConsoleCommandNames.Add(TEXT("stat MemoryStaticMesh"));
	Group.ConsoleCommandNames.Add(TEXT("stat MemoryAllocator"));

	ConsoleCommandGroups.Add(Group);
	Group.ConsoleCommandNames.Empty();
}

void UGameDebugMenuSettings::SetupCategoryShowDebug()
{
	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 5; /* Show Debug */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Toggle VisualizeMovement"));
	Pair.Description              = FText::FromString(TEXT("キャラクターの加速度や移動モードを表示させる"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("p.VisualizeMovement 1");
	Pair.SecondConsoleCommandName = TEXT("p.VisualizeMovement 0");
	ConsoleCommandPairs.Add(Pair);

	Pair.Title                    = FText::FromString(TEXT("Toggle VisualizeLODs"));
	Pair.Description              = FText::FromString(TEXT("SkinnedMeshComponentで適応してるLODの確認\nボーン数、頂点数などが確認可能(Shipping / Test以外のビルドで有効)\nLOD0:White LOD1:Green LOD2:Yellow LOD3:Red Other:Purple"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("a.VisualizeLODs 1");
	Pair.SecondConsoleCommandName = TEXT("a.VisualizeLODs 0");
	ConsoleCommandPairs.Add(Pair);

	Pair.Title                    = FText::FromString(TEXT("Toggle VisualizeOccludedPrimitives"));
	Pair.Description              = FText::FromString(TEXT("オクルードされたアクタを視覚化する"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("r.VisualizeOccludedPrimitives 1");
	Pair.SecondConsoleCommandName = TEXT("r.VisualizeOccludedPrimitives 0");
	ConsoleCommandPairs.Add(Pair);

	Pair.Title                    = FText::FromString(TEXT("Toggle LODColoration"));
	Pair.Description              = FText::FromString(TEXT("PrimitiveComponentのLODレベル毎の色で確認\nLOD0:White LOD1:Red LOD2:Green LOD3:Blue LOD4:Yellow LOD5:Fuchisia LOD6:Cyan LOD7:Purple"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("ShowFlag.LODColoration 1");
	Pair.SecondConsoleCommandName = TEXT("ShowFlag.LODColoration 0");
	ConsoleCommandPairs.Add(Pair);

	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 5; /* Show Debug */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;
	
	Single.Title              = FText::FromString(TEXT("Toggle ShowDebug"));
	Single.Description        = FText::FromString(TEXT("HUDを使用したデバック情報を画面に表示するかを切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebug");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug NextDebugTarget"));
	Single.Description        = FText::FromString(TEXT("デバック情報の表示対象アクターを切り替える（次へ進む）"));
	Single.ConsoleCommandName = TEXT("NextDebugTarget");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug PreviousDebugTarget"));
	Single.Description        = FText::FromString(TEXT("デバック情報の表示対象アクターを切り替える（戻る）"));
	Single.ConsoleCommandName = TEXT("PreviousDebugTarget");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Reset"));
	Single.Description        = FText::FromString(TEXT("HUDを使用したデバック情報の表示項目をリセットする"));
	Single.ConsoleCommandName = TEXT("ShowDebug Reset");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Input"));
	Single.Description        = FText::FromString(TEXT("入力情報を表示する"));
	Single.ConsoleCommandName = TEXT("ShowDebug Input");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Enhancedinput"));
	Single.Description        = FText::FromString(TEXT("Enhancedinputの入力情報を表示する"));
	Single.ConsoleCommandName = TEXT("Showdebug Enhancedinput");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("ShowDebug Collision"));
	Single.Description        = FText::FromString(TEXT("対象アクターのコリジョン情報を表示する"));
	Single.ConsoleCommandName = TEXT("ShowDebug COLLISION");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Physics"));
	Single.Description        = FText::FromString(TEXT("対象アクターのPhysics関係の情報を表示する"));
	Single.ConsoleCommandName = TEXT("ShowDebug PHYSICS");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Forcefeedback"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("ShowDebug FORCEFEEDBACK");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Camera"));
	Single.Description        = FText::FromString(TEXT("アクティブなカメラの情報を表示する"));
	Single.ConsoleCommandName = TEXT("ShowDebug CAMERA");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("ShowDebug Animation"));
	Single.Description        = FText::FromString(TEXT("対象アクターのSkeletalMeshのアニメーション情報を表示する"));
	Single.ConsoleCommandName = TEXT("ShowDebug ANIMATION");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle Graph"));
	Single.Description        = FText::FromString(TEXT("アニムグラフの内容を表示を切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory GRAPH");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle FullGraph"));
	Single.Description        = FText::FromString(TEXT("アニムグラフの内容をすべて表示するかを切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory FULLGRAPH");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle Curves"));
	Single.Description        = FText::FromString(TEXT("Curve情報を表示するか切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory CURVES");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle Montages"));
	Single.Description        = FText::FromString(TEXT("モンタージュ情報を表示するか切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory MONTAGES");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle Notifies"));
	Single.Description        = FText::FromString(TEXT("アニメーション通知情報を表示するか切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory NOTIFIES");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle SyncGroups"));
	Single.Description        = FText::FromString(TEXT("SyncGroup情報を表示するか切り替える"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory SYNCGROUPS");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowDebug Animation Toggle FullBlendSpaceDisplay"));
	Single.Description        = FText::FromString(TEXT("ブレンドスペースの重み値の表示を切り替える（SYNCGROUPSが表示されるとき反映）"));
	Single.ConsoleCommandName = TEXT("ShowDebugToggleSubCategory FULLBLENDSPACEDISPLAY");
	ConsoleCommandNames.Add(Single);

}

void UGameDebugMenuSettings::SetupCategoryViewMode()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 6; /* ViewMode */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Lit"));
	Single.Description        = FText::FromString(TEXT("シーン全てのライティングがある状態\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode lit");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Unlit"));
	Single.Description        = FText::FromString(TEXT("シーン全てのライティングを切った状態\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode unlit");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Wireframe"));
	Single.Description        = FText::FromString(TEXT("シーン内のすべてのポリゴンエッジを表示"));
	Single.ConsoleCommandName = TEXT("viewmode Wireframe");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Detaillighting"));
	Single.Description        = FText::FromString(TEXT("シーン全体に中間色のマテリアルをアクティベートする\n基本色が暗すぎたり、ライティングが不明瞭になっている場合の分離に有効\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode lit_detaillighting");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Lighting Only"));
	Single.Description        = FText::FromString(TEXT("ライティングにのみ影響を受ける中間色のマテリアルを表示\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode lightingonly");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Reflection"));
	Single.Description        = FText::FromString(TEXT("反射の表示\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode ReflectionOverride");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Light Complexity"));
	Single.Description        = FText::FromString(TEXT("ジオメトリに影響を与える非静的ライトの数を表示\nライトがサーフェスに影響を与えるほど、描画負荷が高くなる\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode lightcomplexity");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Lightmap Density"));
	Single.Description        = FText::FromString(TEXT("テクスチャ マッピングされるオブジェクトのライトマップ密度を表示\n理想的/最大限の密度設定別に色分けして、実際のライトマップ テクセルへマッピングするグリッドを表示\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode LightMapDensity");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Shader Complexity"));
	Single.Description        = FText::FromString(TEXT("シーンの各ピクセルの計算に使用しているシェーダ命令数を視覚化する\nエディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode shadercomplexity");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShaderComplexity With Quadoverdraw"));
	Single.Description        = FText::FromString(TEXT("エディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode ShaderComplexityWithQuadOverdraw");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Quadoverdraw"));
	Single.Description        = FText::FromString(TEXT("エディターでかつStandaloneのみ有効\n(Package環境で使用したい場合は「r.ForceDebugViewModes」を1に)"));
	Single.ConsoleCommandName = TEXT("viewmode Quadoverdraw");
	ConsoleCommandNames.Add(Single);

	/* EditorOnly */

	Single.Title = FText::FromString(TEXT("Stationary Light Overlap"));
	Single.Description = FText::FromString(TEXT("Stationary Lightの重なりを視覚化する\nエディターでのみ有効"));
	Single.ConsoleCommandName = TEXT("show StationaryLightOverlap");
	EditorOnlyConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Visualize Buffer"));
	Single.Description        = FText::FromString(TEXT("各レンダーバッファの可視化\nエディターでのみ有効"));
	Single.ConsoleCommandName = TEXT("show VisualizeBuffer");
	EditorOnlyConsoleCommandNames.Add(Single);

	/* EditorOnly */

	Single.Title              = FText::FromString(TEXT("Collision"));
	Single.Description        = FText::FromString(TEXT("コリジョン（非）表示"));
	Single.ConsoleCommandName = TEXT("show collision");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Collision Pawn"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show CollisionPawn");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Collision Visibility"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show CollisionVisibility");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Bounds"));
	Single.Description        = FText::FromString(TEXT("各Actorのバウンディングボックスの表示"));
	Single.ConsoleCommandName = TEXT("show bounds");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Shadow Frustums"));
	Single.Description        = FText::FromString(TEXT("動的な影を生成しているフラスタムを表示"));
	Single.ConsoleCommandName = TEXT("show ShadowFrustums");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Dynamic Shadows"));
	Single.Description        = FText::FromString(TEXT("全ての動的シャドウを切り替え (シャドウマップ レンダリングとシャドウ フィルタリング / プロジェクション)"));
	Single.ConsoleCommandName = TEXT("show DynamicShadows");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Volumes"));
	Single.Description        = FText::FromString(TEXT("AVolume継承アクターのボリュームを(非)表示"));
	Single.ConsoleCommandName = TEXT("show Volumes");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Navigation"));
	Single.Description        = FText::FromString(TEXT("ナビメッシュの(非)表示"));
	Single.ConsoleCommandName = TEXT("show Navigation");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Deferred Lighting"));
	Single.Description        = FText::FromString(TEXT("すべてのディファード ライティング パスを切り替え"));
	Single.ConsoleCommandName = TEXT("show DeferredLighting");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("DirectionalLights"));
	Single.Description        = FText::FromString(TEXT("ディレクショナルライトの表示切り替え"));
	Single.ConsoleCommandName = TEXT("show DirectionalLights");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("PointLights"));
	Single.Description        = FText::FromString(TEXT("ポイントライトの表示切り替え"));
	Single.ConsoleCommandName = TEXT("show PointLights");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("SpotLights"));
	Single.Description        = FText::FromString(TEXT("スポットライトの表示切り替え"));
	Single.ConsoleCommandName = TEXT("show SpotLights");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Rendering"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Rendering");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("SkyLighting"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show SkyLighting");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Decals"));
	Single.Description        = FText::FromString(TEXT("デカールの表示切り替え"));
	Single.ConsoleCommandName = TEXT("show Decals");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Post Processing"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show PostProcessing");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Particles"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Particles");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Specular"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Specular");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Translucency"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Translucency");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Diffuse"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Diffuse");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ReflectionEnvironment"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show ReflectionEnvironment");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Refraction"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Refraction");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AmbientOcclusion"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show AmbientOcclusion");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("GlobalIllumination"));
	Single.Description        = FText::FromString(TEXT("ベイクされた、動的な間接ライティングを切り替え"));
	Single.ConsoleCommandName = TEXT("show GlobalIllumination");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LightFunctions"));
	Single.Description        = FText::FromString(TEXT("ライト関数のレンダリングを切り替え"));
	Single.ConsoleCommandName = TEXT("show LightFunctions");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("DepthOfField"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show DepthOfField");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AntiAliasing"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show AntiAliasing");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Bloom"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Bloom");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("StaticMeshes"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show StaticMeshes");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("SkeletalMeshes"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show SkeletalMeshes");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Landscape"));
	Single.Description        = FText::FromString(TEXT(""));
	Single.ConsoleCommandName = TEXT("show Landscape");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Tessellation"));
	Single.Description        = FText::FromString(TEXT("テッセレーションを切り替え (テッセレーション シェーダーは実行されたまま)"));
	Single.ConsoleCommandName = TEXT("show Tessellation");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("VisualizeVolumetricLightmap"));
	Single.Description        = FText::FromString(TEXT("ボリュームライトマップの可視化"));
	Single.ConsoleCommandName = TEXT("show VisualizeVolumetricLightmap");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ScreenSpaceReflections"));
	Single.Description        = FText::FromString(TEXT("スクリーン スペースの反射を切り替え"));
	Single.ConsoleCommandName = TEXT("show ScreenSpaceReflections");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Shader Complexity"));
	Single.Description        = FText::FromString(TEXT("シーンの各ピクセルを計算するために使用されているシェーダ命令の数を視覚化"));
	Single.ConsoleCommandName = TEXT("show ShaderComplexity");
	ConsoleCommandNames.Add(Single);

	/* EditorOnly */
	
	Single.Title              = FText::FromString(TEXT("Materials"));
	Single.Description        = FText::FromString(TEXT("マテリアルのオンオフ\nエディターでのみ有効"));
	Single.ConsoleCommandName = TEXT("show Materials");
	EditorOnlyConsoleCommandNames.Add(Single);

	/* EditorOnly */
}

void UGameDebugMenuSettings::SetupCategoryScalability()
{
	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 7; /* Scalability */
	Number.CommandNetType         = EGDMConsoleCommandNetType::LocalOnly;
	Number.PreConsoleCommandName  = TEXT("sg.");
	Number.PostConsoleCommandName = TEXT(" ");
	Number.DefaultValue = 0.0f;

	Number.Title                            = FText::FromString(TEXT("ViewDistanceQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("ViewDistanceQuality");
	Number.ConsoleVariableName				= TEXT("sg.ViewDistanceQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("AntiAliasingQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("AntiAliasingQuality");
	Number.ConsoleVariableName				= TEXT("sg.AntiAliasingQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 6.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("PostProcessQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("PostProcessQuality");
	Number.ConsoleVariableName				= TEXT("sg.PostProcessQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("ShadowQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("ShadowQuality");
	Number.ConsoleVariableName				= TEXT("sg.ShadowQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("TextureQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("TextureQuality");
	Number.ConsoleVariableName				= TEXT("sg.TextureQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);
	
	Number.Title                            = FText::FromString(TEXT("EffectsQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("EffectsQuality");
	Number.ConsoleVariableName				= TEXT("sg.EffectsQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("FoliageQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("FoliageQuality");
	Number.ConsoleVariableName				= TEXT("sg.FoliageQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("ShadingQuality"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("ShadingQuality");
	Number.ConsoleVariableName				= TEXT("sg.ShadingQuality");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 3.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);


	Number.PreConsoleCommandName = TEXT("r.");
	Number.PostConsoleCommandName = TEXT(" ");

	Number.Title                            = FText::FromString(TEXT("ScreenPercentage"));
	Number.Description                      = FText::FromString(TEXT(""));
	Number.ConsoleCommandName               = TEXT("ScreenPercentage");
	Number.ConsoleVariableName				= TEXT("r.ScreenPercentage");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 10.0f;
	Number.UIConfigInfo.Range.MaxValue      = 200.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 5.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("DynamicRes.OperationMode"));
	Number.Description                      = FText::FromString(TEXT("動的解像度の使用方法の切り替え\n 0 = 無効\n 1 = GameUserSettingsの設定に基づいて有効化\n2 = GameUserSettingsを考慮せず有効化"));
	Number.ConsoleCommandName               = TEXT("DynamicRes.OperationMode");
	Number.ConsoleVariableName				= TEXT("r.DynamicRes.OperationMode");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 2.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("MaterialQualityLevel"));
	Number.Description                      = FText::FromString(TEXT("マテリアル品質レベルの変更\n0 = Low: 1 = High: 2 = Medium"));
	Number.ConsoleCommandName               = TEXT("MaterialQualityLevel");
	Number.ConsoleVariableName				= TEXT("r.MaterialQualityLevel");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 2.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 1.0f;
	ConsoleCommandNumbers.Add(Number);

	Number.PreConsoleCommandName = TEXT("r.DOF.");

	Number.Title                            = FText::FromString(TEXT("DOF - MaxBackgroundRadius"));
	Number.Description                      = FText::FromString(TEXT("水平スクリーン空間のバックグラウンド ブラー半径の最大サイズ"));
	Number.ConsoleCommandName               = TEXT("Kernel.MaxBackgroundRadius");
	Number.ConsoleVariableName              = TEXT("r.DOF.Kernel.MaxBackgroundRadius");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 0.001f;
	Number.UIConfigInfo.MaxChangeAmount     = 0.005f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("DOF - MaxForegroundRadius"));
	Number.Description                      = FText::FromString(TEXT("水平スクリーン空間のフォアグラウンド ブラー半径の最大サイズ"));
	Number.ConsoleCommandName               = TEXT("Kernel.MaxForegroundRadius");
	Number.ConsoleVariableName              = TEXT("r.DOF.Kernel.MaxForegroundRadius");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue		= 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 0.001f;
	Number.UIConfigInfo.MaxChangeAmount     = 0.005f;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("DOF - MaxSpriteRatio"));
	Number.Description                      = FText::FromString(TEXT("スプライトとしての散乱ピクセル クワッドの最大比率\nDOF の散乱の上限をコントロールするために便利\n1では100％のピクセルクワッドを散乱可能に"));
	Number.ConsoleCommandName               = TEXT("Scatter.MaxSpriteRatio");
	Number.ConsoleVariableName              = TEXT("r.DOF.Scatter.MaxSpriteRatio");
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 0.01f;
	Number.UIConfigInfo.MaxChangeAmount     = 0.05f;
	ConsoleCommandNumbers.Add(Number);

	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 7; /* Scalability */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Toggle VSync"));
	Pair.Description              = FText::FromString(TEXT("垂直同期の切り替え"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("r.VSync 0");
	Pair.SecondConsoleCommandName = TEXT("r.VSync 1");
	ConsoleCommandPairs.Add(Pair);
}

void UGameDebugMenuSettings::SetupCategoryFreeze()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 8; /* Freeze */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.ClickedEvent = EGDMConsoleCommandClickedEvent::Non;

	Single.Title              = FText::FromString(TEXT("Freeze Rendering"));
	Single.Description        = FText::FromString(TEXT("レベル内にあるオクルードされたアクタ\nおよび表示されているアクタの現在のレンダリングの状態を一時停止 / 解除する"));
	Single.ConsoleCommandName = TEXT("FreezeRendering");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("FreezeParticleSimulation"));
	Single.Description        = FText::FromString(TEXT("レベル内のすべての CPU スプライト パーティクル シミュレーション\nの一時停止 / 解除する"));
	Single.ConsoleCommandName = TEXT("FX.FreezeParticleSimulation");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("FreezeGPUSimulation"));
	Single.Description        = FText::FromString(TEXT("レベル内のすべての GPU スプライト パーティクル シミュレーション\nの一時停止 / 解除する"));
	Single.ConsoleCommandName = TEXT("FX.FreezeGPUSimulation");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandPair Pair;
	Pair.CategoryIndex = 8; /* Freeze */
	Pair.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Pair.Title                    = FText::FromString(TEXT("Foliage Freeze / Unfreeze"));
	Pair.Description              = FText::FromString(TEXT("レベル内でオクルードおよび表示された\nペイントされたフォリッジ クラスタの現在のレンダリングの状態を一時停止 / 解除する"));
	Pair.ClickedEvent             = EGDMConsoleCommandClickedEvent::Non;
	Pair.FirstConsoleCommandName  = TEXT("Foliage.Freeze");
	Pair.SecondConsoleCommandName = TEXT("Foliage.Unfreeze");
	ConsoleCommandPairs.Add(Pair);

}

void UGameDebugMenuSettings::SetupCategoryDumpLogs()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 9; /* Dump Logs */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Obj List"));
	Single.Description        = FText::FromString(TEXT("OutputLogウィンドウに各オブジェクトの数と使用メモリ量を出力する"));
	Single.ConsoleCommandName = TEXT("Obj List");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Obj List Forget"));
	Single.Description        = FText::FromString(TEXT("Obj Listで取得したObjectを記憶し\n これ以降のObj Listでは差分が表示されるようになる"));
	Single.ConsoleCommandName = TEXT("Obj List Forget");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Obj List Remember"));
	Single.Description        = FText::FromString(TEXT("Obj List Forget で記憶したオブジェクトをリセットする"));
	Single.ConsoleCommandName = TEXT("Obj List Remember");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("MemReport"));
	Single.Description        = FText::FromString(TEXT("YourGame/Saved/Profiling/MemReports以下にメモリ使用ログを出力する"));
	Single.ConsoleCommandName = TEXT("memreport");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("MemReport Full"));
	Single.Description        = FText::FromString(TEXT("MemReportのより詳細にログを出力します"));
	Single.ConsoleCommandName = TEXT("memreport -full");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Dump Ticks"));
	Single.Description        = FText::FromString(TEXT("Level上に存在するActorやComponentのTickが有効なものをリストアップしログ表示する"));
	Single.ConsoleCommandName = TEXT("dumpticks Enable");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Loadtimes Dumpreport"));
	Single.Description        = FText::FromString(TEXT("ロードされたファイルのロード時間を降順でログ出力する\n(loadtimes.resetでタイミング調整可能)"));
	Single.ConsoleCommandName = TEXT("loadtimes.dumpreport");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Dump Hitches"));
	Single.Description        = FText::FromString(TEXT("t.HitchFrameTimeThreshold に基づいて処理落ちが検出されるたびに\nログへ書き込まれる"));
	Single.ConsoleCommandName = TEXT("stat DumpHitches");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Log Garbage"));
	Single.Description        = FText::FromString(TEXT("GCが起きたときの検索、削除コストをログに表示する"));
	Single.ConsoleCommandName = TEXT("log LogGarbage log");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Log List"));
	Single.Description        = FText::FromString(TEXT("各ログカテゴリの一覧と、各カテゴリの現在の設定を確認できる"));
	Single.ConsoleCommandName = TEXT("log list");
	ConsoleCommandNames.Add(Single);

	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 9; /* Dump Logs */
	Number.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Number.Title                            = FText::FromString(TEXT("Hitch Frame Time Threshold"));
	Number.Description                      = FText::FromString(TEXT("DumpHitches実行時ログに出力するかどうかのしきい値（ミリ秒）\n この値よりも処理に時間がかかるものがログに出力されるようになる"));
	Number.PreConsoleCommandName            = TEXT("t.");
	Number.ConsoleCommandName               = TEXT("HitchFrameTimeThreshold");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("t.HitchFrameTimeThreshold");
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.bUseMax       = false;
	Number.UIConfigInfo.Range.MinValue      = 10.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 5.0f;
	Number.UIConfigInfo.MaxChangeAmount     = 10.0f;
	Number.DefaultValue                     = FEnginePerformanceTargets::GetHitchFrameTimeThresholdMS();
	ConsoleCommandNumbers.Add(Number);
}

void UGameDebugMenuSettings::SetupCategoryNetwork()
{
	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 10; /* Network */
	Number.ConsoleVariableName = TEXT("");

	/* NetEmulation系は FConsoleCommandWithWorldAndArgsDelegate で定義されてるため
	 * IConsoleObjectから値が取れない。。。 */

	Number.Title                            = FText::FromString(TEXT("PktLag"));
	Number.Description                      = FText::FromString(TEXT("パケット送信を遅らせる(ミリ秒)"));
	Number.PreConsoleCommandName            = TEXT("NetEmulation.");
	Number.ConsoleCommandName               = TEXT("PktLag");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	Number.UIConfigInfo.Range.bUseMax       = false;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 10;
	Number.UIConfigInfo.MaxChangeAmount     = 100;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("PktLagVariance"));
	Number.Description                      = FText::FromString(TEXT("パケット送信を指定範囲内でランダムに遅らせる(±ミリ秒)"));
	Number.PreConsoleCommandName            = TEXT("NetEmulation.");
	Number.ConsoleCommandName               = TEXT("PktLagVariance");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	Number.UIConfigInfo.Range.bUseMax       = false;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 10;
	Number.UIConfigInfo.MaxChangeAmount     = 100;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("PktLoss"));
	Number.Description                      = FText::FromString(TEXT("一定の確率でパケットを送信しないようにする(0～100％)"));
	Number.PreConsoleCommandName            = TEXT("NetEmulation.");
	Number.ConsoleCommandName               = TEXT("PktLoss");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 100.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 10;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("PktOrder"));
	Number.Description                      = FText::FromString(TEXT("パケットをバッファリングして送信順序をランダムにする（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("NetEmulation.");
	Number.ConsoleCommandName               = TEXT("PktOrder");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("PktDup"));
	Number.Description                      = FText::FromString(TEXT("パケット送信時に指定確率で重複送信する(0～100％)"));
	Number.PreConsoleCommandName            = TEXT("NetEmulation.");
	Number.ConsoleCommandName               = TEXT("PktDup");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::ServerAll;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 100.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 10;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("RPC Debug"));
	Number.Description                      = FText::FromString(TEXT("すべてのRPC Bunchをログ表示（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName               = TEXT("RPC");
	Number.PostConsoleCommandName           = TEXT(".Debug ");
	Number.ConsoleVariableName				= TEXT("net.RPC.Debug");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("Reliable Debug"));
	Number.Description                      = FText::FromString(TEXT("すべてのReliable Bunchのログ表示（０＝出力なし、１＝送信時のみ出力、２＝更新時に出力）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName               = TEXT("Reliable");
	Number.PostConsoleCommandName           = TEXT(".Debug ");
	Number.ConsoleVariableName				= TEXT("net.Reliable.Debug");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 2.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("NetShowCorrections"));
	Number.Description                      = FText::FromString(TEXT("クライアント (またはサーバーへ送信) がネットワーク修正を受け取った時期を確認できるようにする（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("p.");
	Number.ConsoleCommandName               = TEXT("NetShowCorrections");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("p.NetShowCorrections");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("Debug Draw"));
	Number.Description                      = FText::FromString(TEXT("ネットワークのdormancyとrelevancyの情報を表示（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName               = TEXT("DebugDraw");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("net.DebugDraw");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("Context Debug"));
	Number.Description                      = FText::FromString(TEXT("Replication情報にデバッグ用文字列を設定してログに詳細な情報を表示（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName               = TEXT("ContextDebug");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("net.ContextDebug");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("Dormancy Enable"));
	Number.Description                      = FText::FromString(TEXT("頻繁に更新されるアクターのCPUおよび帯域幅のオーバーヘッドを削減するネットワーク休止システムの有効化（１＝有効、０＝無効）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName               = TEXT("DormancyEnable");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("net.DormancyEnable");
	Number.DefaultValue                     = 1.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 1.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 1;
	Number.UIConfigInfo.MaxChangeAmount     = 1;
	ConsoleCommandNumbers.Add(Number);

	Number.Title                            = FText::FromString(TEXT("Debug Draw CullDistance"));
	Number.Description                      = FText::FromString(TEXT("localのViewから離れているActorの休止状態を描画する\nこの距離（World単位）より近いものが描画対象（net.DebugDrawが有効かつ 0 以上指定で動作）"));
	Number.PreConsoleCommandName            = TEXT("net.");
	Number.ConsoleCommandName				= TEXT("DebugDrawCullDistance");	
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.ConsoleVariableName				= TEXT("net.DebugDrawCullDistance");
	Number.DefaultValue                     = 0.0f;
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = false;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 0.0f;
	Number.UIConfigInfo.Range.MaxValue      = 0.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 50;
	Number.UIConfigInfo.MaxChangeAmount     = 100;
	ConsoleCommandNumbers.Add(Number);


	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 10; /* Network */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;
	
	Single.Title              = FText::FromString(TEXT("NET"));
	Single.Description        = FText::FromString(TEXT("ネットワーキング システムに関する統計情報を表示"));
	Single.ConsoleCommandName = TEXT("stat NET");

	Single.Title = FText::FromString(TEXT("Online"));
	Single.Description = FText::FromString(TEXT("オンライン システムのカウンターを表示"));
	Single.ConsoleCommandName = TEXT("stat Online");

	Single.Title              = FText::FromString(TEXT("Dump Relevant Actors"));
	Single.Description        = FText::FromString(TEXT("次回ネットワーク更新時に関連するActorの情報を出力"));
	Single.ConsoleCommandName = TEXT("net.DumpRelevantActors");

	Single.Title              = FText::FromString(TEXT("List Actor Channels"));
	Single.Description        = FText::FromString(TEXT("ActorChannelのリスト一覧表示"));
	Single.ConsoleCommandName = TEXT("net.ListActorChannels");

	Single.Title              = FText::FromString(TEXT("List Net GUIDs"));
	Single.Description        = FText::FromString(TEXT("NetGUIDのリスト一覧表示"));
	Single.ConsoleCommandName = TEXT("net.ListNetGUIDs");

	Single.Title              = FText::FromString(TEXT("List Net GUID Exports"));
	Single.Description        = FText::FromString(TEXT("NetGUIDとエクスポート回数のリスト一覧表示"));
	Single.ConsoleCommandName = TEXT("net.ListNetGUIDExports");
	
}

void UGameDebugMenuSettings::SetupCategorySound()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 11; /* Sounds */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;


	FGDMConsoleCommandGroup Group;
	Group.CategoryIndex = 11; /* Sounds */
	Group.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;


	Group.Title = FText::FromString(TEXT("Sounds"));
	Group.Description = FText::FromString(TEXT("アクティブなサウンドキューとサウンドウェーブ、SoundMixeを表示"));
	Group.ConsoleCommandNames.Add("stat SoundMixes");
	Group.ConsoleCommandNames.Add("stat SoundWaves");
	Group.ConsoleCommandNames.Add("stat SoundCues");
	Group.ConsoleCommandNames.Add("stat Sounds");

	ConsoleCommandGroups.Add(Group);
	Group.ConsoleCommandNames.Empty();
}

void UGameDebugMenuSettings::SetupCategoryAbilitySystem()
{
	/* https://dev.epicgames.com/community/learning/tutorials/Y477/unreal-engine-gameplay-ability-system-debugging-tools */
	
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 12; /* Ability System */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.Title              = FText::FromString(TEXT("Show abilitysystem"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemのデバック表示をする"));
	Single.ConsoleCommandName = TEXT("showdebug abilitysystem");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Close abilitysystem"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemの非表示にする"));
	Single.ConsoleCommandName = TEXT("showdebug");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AbilitySystem NextCategory"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemのデバック表示がされてる場合表示内容（所持アビリティ、エフェクトなど）を切り替える"));
	Single.ConsoleCommandName = TEXT("abilitySystem.debug.nextCategory");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AbilitySystem NextTarget"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemのデバック表示対象を変更する(次へ移る)"));
//	Single.ConsoleCommandName = TEXT("abilitysystem.debug.nexttarget");
	Single.ConsoleCommandName = TEXT("NextDebugTarget");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AbilitySystem PrevTarget"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemのデバック表示を対象を変更する(１つ戻る)"));
//	Single.ConsoleCommandName = TEXT("abilitysystem.debug.prevtarget");
	Single.ConsoleCommandName = TEXT("PreviousDebugTarget");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AbilitySystem DebugBasicHUD"));
	Single.Description        = FText::FromString(TEXT("操作キャラのAttribute情報を表示"));
	Single.ConsoleCommandName = TEXT("AbilitySystem.DebugBasicHUD");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("AbilitySystem DebugAbilityTags"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemで所持するタグ情報をアクター位置にすべて表示"));
	Single.ConsoleCommandName = TEXT("AbilitySystem.DebugAbilityTags");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("AbilitySystem DebugAttribute"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemで所持するAttribute情報をアクター位置にすべて表示"));
	Single.ConsoleCommandName = TEXT("AbilitySystem.DebugAttribute");
	ConsoleCommandNames.Add(Single);
	
	Single.Title              = FText::FromString(TEXT("AbilitySystem ClearDebugAttributes"));
	Single.Description        = FText::FromString(TEXT("AbilitySystemで所持するAttribute情報をクリアする"));
	Single.ConsoleCommandName = TEXT("AbilitySystem.ClearDebugAttributes");
	ConsoleCommandNames.Add(Single);
	
	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 12; /* Ability System */

	Number.Title                            = FText::FromString(TEXT("AbilitySystem.DebugDrawMaxDistance"));
	Number.Description                      = FText::FromString(TEXT("アクター位置に表示するAbilitySystemのデバック情報の表示距離を設定する"));
	Number.PreConsoleCommandName            = TEXT("");
	Number.ConsoleCommandName               = TEXT("AbilitySystem.DebugDrawMaxDistance");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue						= 2048.0f;
	Number.ConsoleVariableName				= TEXT("AbilitySystem.DebugDrawMaxDistance");
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = false;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 10.0f;
	Number.UIConfigInfo.Range.MaxValue      = 0.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 10;
	Number.UIConfigInfo.MaxChangeAmount     = 100;
	ConsoleCommandNumbers.Add(Number);
}

void UGameDebugMenuSettings::SetupCategoryOther()
{
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 13; /* Other */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	Single.ClickedEvent		  = EGDMConsoleCommandClickedEvent::Non;

	Single.Title              = FText::FromString(TEXT("RestartLevel"));
	Single.Description        = FText::FromString(TEXT("レベルの再読込み"));
	Single.ConsoleCommandName = TEXT("RestartLevel");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("ShowLog"));
	Single.Description        = FText::FromString(TEXT("コンソールウィンドウの（非）表示する"));
	Single.ConsoleCommandName = TEXT("ShowLog");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("Try Garbage Collection"));
	Single.Description        = FText::FromString(TEXT("ガーベジコレクションを即時実行し、定期実行タイマーをリセットする"));
	Single.ConsoleCommandName = TEXT("Obj trygc");
	ConsoleCommandNames.Add(Single);

	Single.CommandNetType     = EGDMConsoleCommandNetType::LocalOnly;
	Single.Title              = FText::FromString(TEXT("Exit Game"));
	Single.Description        = FText::FromString(TEXT("ゲームを即終了させる"));
	Single.ConsoleCommandName = TEXT("exit");
	ConsoleCommandNames.Add(Single);


	FGDMConsoleCommandNumber Number;
	Number.CategoryIndex = 13; /* Other */

	Number.Title                            = FText::FromString(TEXT("Change Max FPS"));
	Number.Description                      = FText::FromString(TEXT("FPSの変更"));
	Number.PreConsoleCommandName            = TEXT("t.");
	Number.ConsoleCommandName               = TEXT("MaxFPS");
	Number.PostConsoleCommandName           = TEXT(" ");
	Number.DefaultValue						= 60.0f;
	Number.ConsoleVariableName				= TEXT("t.MaxFPS");
	Number.CommandNetType                   = EGDMConsoleCommandNetType::LocalOnly;
	Number.UIConfigInfo.Range.bUseMax       = true;
	Number.UIConfigInfo.Range.bUseMin       = true;
	Number.UIConfigInfo.Range.MinValue      = 15.0f;
	Number.UIConfigInfo.Range.MaxValue      = 200.0f;
	Number.UIConfigInfo.DefaultChangeAmount = 5;
	Number.UIConfigInfo.MaxChangeAmount     = 10;
	ConsoleCommandNumbers.Add(Number);

}

void UGameDebugMenuSettings::SetupCategoryLogVerbosity()
{
	/* Engine\Source\Runtime\Core\Public\Logging\LogVerbosity.hのELogVerbosity参照 */
	FGDMConsoleCommandSingle Single;
	Single.CategoryIndex = 14; /* Log Verbosity */
	Single.CommandNetType = EGDMConsoleCommandNetType::LocalOnly;

	/************************************************************************/
	/* LogTemp                                                              */
	/************************************************************************/

	Single.Title = FText::FromString(TEXT("LogTemp - Display"));
	Single.Description = FText::FromString(TEXT("LogTempのVerbosityを「Display」に変更"));
	Single.ConsoleCommandName = TEXT("log LogTemp Display");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogTemp - Log"));
	Single.Description        = FText::FromString(TEXT("LogTempのVerbosityを「Log」に変更"));
	Single.ConsoleCommandName = TEXT("log LogTemp Log");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogTemp - Verbose"));
	Single.Description        = FText::FromString(TEXT("LogTempのVerbosityを「Verbose」に変更"));
	Single.ConsoleCommandName = TEXT("log LogTemp Verbose");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogTemp - VeryVerbose"));
	Single.Description        = FText::FromString(TEXT("LogTempのVerbosityを「VeryVerbose」に変更"));
	Single.ConsoleCommandName = TEXT("log LogTemp VeryVerbose");
	ConsoleCommandNames.Add(Single);

	/************************************************************************/
	/* LogAbilitySystem												  */
	/************************************************************************/

	Single.Title              = FText::FromString(TEXT("LogAbilitySystem - Display"));
	Single.Description        = FText::FromString(TEXT("LogAbilitySystemのVerbosityを「Display」に変更"));
	Single.ConsoleCommandName = TEXT("log LogAbilitySystem Display");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogAbilitySystem - Log"));
	Single.Description        = FText::FromString(TEXT("LogAbilitySystemのVerbosityを「Log」に変更"));
	Single.ConsoleCommandName = TEXT("log LogAbilitySystem Log");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogAbilitySystem - Verbose"));
	Single.Description        = FText::FromString(TEXT("LogAbilitySystemのVerbosityを「Verbose」に変更"));
	Single.ConsoleCommandName = TEXT("log LogAbilitySystem Verbose");
	ConsoleCommandNames.Add(Single);

	Single.Title              = FText::FromString(TEXT("LogAbilitySystem - VeryVerbose"));
	Single.Description        = FText::FromString(TEXT("LogAbilitySystemのVerbosityを「VeryVerbose」に変更"));
	Single.ConsoleCommandName = TEXT("log LogAbilitySystem VeryVerbose");
	ConsoleCommandNames.Add(Single);
}
