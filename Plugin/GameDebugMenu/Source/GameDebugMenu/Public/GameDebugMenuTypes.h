/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Framework/Application/NavigationConfig.h"
#include <Internationalization/StringTable.h>
#include "GameDebugMenuTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGDM, Log, All);

/** NavigationをきるためだけのConfig */
class FGDMNavigationConfig : public FNavigationConfig
{
public:
	FGDMNavigationConfig()
		:FNavigationConfig()
	{
		/** DebugMenuでは使用しない */
		AnalogHorizontalKey = EKeys::Invalid;
		AnalogVerticalKey   = EKeys::Invalid;
		KeyEventRules.Empty();
	}

	virtual ~FGDMNavigationConfig() {}
};

/**
*
*/
UENUM(BlueprintType)
enum class EGDMConsoleCommandType : uint8
{
	Non,
	Single,
	Group,
	Pair,
	Number,
};

/**
* メニュー選択後の動作
*/
UENUM(BlueprintType)
enum class EGDMConsoleCommandClickedEvent : uint8
{
	/** 特に何もしない */
	Non,

	/** クリック後メニューを閉じる */
	MenuClose,
};

/**
* プロパティの種類
*/
UENUM(BlueprintType)
enum class EGDMPropertyType : uint8
{
	GDM_Null,
	GDM_Bool,
	GDM_Int,
	GDM_Float,
	GDM_Enum,
	GDM_Byte,
	GDM_String,
	GDM_Vector,
	GDM_Vector2D,
	GDM_Rotator,
};

/**
* プロパティ編集時の最大値、最小値
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMPropertyRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue;

	FGDMPropertyRange()
		: bUseMin(false)
		, bUseMax(false)
		, MinValue(0.0f)
		, MaxValue(0.0f)
	{
	}
};

/**
* プロパティ編集時のUI設定情報
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMPropertyUIConfigInfo
{
	GENERATED_BODY()

	/** 設定範囲 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGDMPropertyRange Range;

	/** 値の変化量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultChangeAmount;

	/** 最大変化量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChangeAmount;

	/** 最大変化量になるまでの時間 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChangeAmountTime;

	FGDMPropertyUIConfigInfo()
		: Range()
		, DefaultChangeAmount(1.0f)
		, MaxChangeAmount(1.0f)
		, MaxChangeAmountTime(1.3f)
	{
	}
};

/**
* コマンドでの実行範囲
*/
UENUM(BlueprintType)
enum class EGDMConsoleCommandNetType : uint8
{
	/** ローカルでのみ実行する */
	LocalOnly,

	/** Serverで全プレイヤーに実行させる */
	ServerAll,
};

/**
* UIでコンソールコマンドを操作するのに使用する情報
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMConsoleCommand
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	EGDMConsoleCommandType Type;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,config)
	EGDMConsoleCommandClickedEvent ClickedEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	uint8 CategoryIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	EGDMConsoleCommandNetType CommandNetType;

	FGDMConsoleCommand();
	virtual ~FGDMConsoleCommand() = default;

	virtual FString BuildCommandIdentifier() const PURE_VIRTUAL(FGDMConsoleCommand::GetCommandId, return FString(););
};

/**
* UIで実行するときに単体で実行する
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMConsoleCommandSingle : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString ConsoleCommandName;

	FGDMConsoleCommandSingle();
	virtual FString BuildCommandIdentifier() const override;
};

/**
* UIで実行するときに同時に実行できるようにするためのグループ
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMConsoleCommandGroup : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FString> ConsoleCommandNames;

	FGDMConsoleCommandGroup();
	virtual FString BuildCommandIdentifier() const override;
};

/**
* UIでトグル操作するときのペアになるコマンド
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMConsoleCommandPair : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString FirstConsoleCommandName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString SecondConsoleCommandName;

	FGDMConsoleCommandPair();
	virtual FString BuildCommandIdentifier() const override;
};

/**
* UIで実行するときに数字指定で実行する
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMConsoleCommandNumber : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString ConsoleCommandName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString PreConsoleCommandName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString PostConsoleCommandName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FGDMPropertyUIConfigInfo UIConfigInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	float DefaultValue;

	/** コマンドの値を取得するときに使用するコンソール名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString ConsoleVariableName;

	FGDMConsoleCommandNumber();
	virtual FString BuildCommandIdentifier() const override;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMMenuCategoryKey
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyName;

public:
	FGDMMenuCategoryKey(uint8 InIndex, FString InKeyName);
	FGDMMenuCategoryKey(uint8 InIndex);
	FGDMMenuCategoryKey();

	bool operator==(FGDMMenuCategoryKey& InOther);
	bool operator!=(FGDMMenuCategoryKey& InOther);
	bool operator<(FGDMMenuCategoryKey& InOther);
	bool operator>(FGDMMenuCategoryKey& InOther);

	FORCEINLINE operator uint8() const { return Index; }
};

USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMGameplayCategoryKey : public FGDMMenuCategoryKey
{
	GENERATED_USTRUCT_BODY()

public:
	FGDMGameplayCategoryKey(uint8 InIndex, FString InKeyName);
	FGDMGameplayCategoryKey(uint8 InIndex);
	FGDMGameplayCategoryKey();
};

/**
* Menuに登録できるプロパティ情報
*/
class GAMEDEBUGMENU_API FGDMObjectPropertyInfo
{
public:
	FGDMGameplayCategoryKey CategoryKey;
	FText Name;
	FText Description;
	TWeakObjectPtr<UObject>	TargetObject;
	FProperty* TargetProperty;
	FName PropertyName;
	TWeakObjectPtr<UEnum> EnumType;
	FGDMPropertyUIConfigInfo ConfigInfo;
	int32 DisplayPriority;
	UScriptStruct* Struct;
	FString PropertySaveKey;

	FGDMObjectPropertyInfo()
		: CategoryKey()
		, Name(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, TargetObject(nullptr)
		, TargetProperty(nullptr)
		, PropertyName(NAME_None)
		, EnumType(nullptr)
		, ConfigInfo()
		, DisplayPriority(0)
		, Struct(nullptr)
		, PropertySaveKey()
	{
	}
};

/**
* Menuに登録できる関数情報
*/
class GAMEDEBUGMENU_API FGDMObjectFunctionInfo
{
public:
	FGDMGameplayCategoryKey CategoryKey;
	FText Name;
	FText Description;
	TWeakObjectPtr<UObject>	TargetObject;
	TWeakObjectPtr<UFunction> TargetFunction;
	FName FunctionName;
	int32 DisplayPriority;
	FString FunctionSaveKey;

	FGDMObjectFunctionInfo()
		: CategoryKey()
		, Name(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, TargetObject(nullptr)
		, TargetFunction(nullptr)
		, FunctionName(NAME_None)
		, DisplayPriority(0)
		, FunctionSaveKey()
	{
	}
};

/**
* 未登録状態のオブジェクト情報
*/
struct GAMEDEBUGMENU_API FGDMPendingObjectData
{
	TWeakObjectPtr<UObject> TargetObject;
	FName TargetName;
	FGDMGameplayCategoryKey CategoryKey;
	FText DisplayPropertyName;
	FText Description;
	FGDMPropertyUIConfigInfo ConfigInfo;
	int32 DisplayPriority;
	FString SaveKey;

	FGDMPendingObjectData()
		: TargetObject(nullptr)
		, TargetName(NAME_None)
		, CategoryKey()
		, DisplayPropertyName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, ConfigInfo()
		, DisplayPriority(0)
		, SaveKey()
	{
	}
};

/**
* 
*/
UENUM(BlueprintType)
enum class EGDMProjectManagementTool : uint8
{
	Redmine,
	Trello,
	Jira,
};

USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMProjectManagementToolSettings
{
	GENERATED_BODY()

	/** 生成したAPIキー */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString AccessKey;

	FGDMProjectManagementToolSettings()
		:AccessKey()
	{
	}
};

/**
* Redmine連携用の情報
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMRedmineSettings : public FGDMProjectManagementToolSettings
{
	GENERATED_BODY()

	/** 接続先ホスト名 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,config)
	FString HostName;

	/** プロジェクト識別ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config, meta = (UIMin = 1, ClampMin = 1))
	int32 ProjectId;

	/** トラッカー名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FText> TrackerNameList;

	/** トラッカー初期値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config, meta = (UIMin = 1, ClampMin = 1))
	int32 DefaultTrackerIndex;

	/** 優先度名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FText> PriorityNameList;

	/** 優先度初期値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config, meta = (UIMin = 1, ClampMin = 1))
	int32 DefaultPriorityIndex;


	FGDMRedmineSettings()
		: Super()
		, HostName(TEXT("localhost"))
		, ProjectId(1)
		, TrackerNameList()
		, DefaultTrackerIndex(0)
		, PriorityNameList()
		, DefaultPriorityIndex(1)
	{
		TrackerNameList.Reset();
		TrackerNameList.Add(FText::FromString(TEXT("バグ")));
		TrackerNameList.Add(FText::FromString(TEXT("機能")));
		TrackerNameList.Add(FText::FromString(TEXT("サポート")));

		PriorityNameList.Reset();
		PriorityNameList.Add(FText::FromString(TEXT("低め")));
		PriorityNameList.Add(FText::FromString(TEXT("通常")));
		PriorityNameList.Add(FText::FromString(TEXT("高め")));
		PriorityNameList.Add(FText::FromString(TEXT("急いで")));
		PriorityNameList.Add(FText::FromString(TEXT("今すぐ")));
	}
};

/**
* Trello連携用の情報
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMTrelloSettings : public FGDMProjectManagementToolSettings
{
	GENERATED_BODY()

	/** Trello用に生成したトークン */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString AccessToken;
	
	/** 追加先リストID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FString> CardListIDs;

	/** 追加先リスト名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FText> CardListNames;


	FGDMTrelloSettings()
		: Super()
		, AccessToken()
		, CardListIDs()
		, CardListNames()
	{
		CardListNames.Add(FText::FromString(TEXT("Trello追加先リスト名")));
	}
};

/**
* Jira連携用の情報
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMJiraSettings : public FGDMProjectManagementToolSettings
{
	GENERATED_BODY()

	/** 接続先ホスト名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString HostName;

	/** プロジェクト識別KEY名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString ProjectKeyName;

	/** ユーザー名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString UserName;

	/** 課題の種類 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FText> IssueTypeList;

	/** 課題の種類初期値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	int32 DefaultIssueTypeIndex;

	/** 優先度名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FText> PriorityNameList;

	/** 優先度初期値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	int32 DefaultPriorityIndex;

	/** 担当者名リスト(Key: accountId Value: 名前) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TMap<FString,FText> AssigneeList;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,config)
	TArray<FString> LabelNameList;

	FGDMJiraSettings()
		: Super()
		, HostName(TEXT("localhost"))
		, ProjectKeyName()
		, UserName()
		, IssueTypeList()
		, DefaultIssueTypeIndex(1)
		, PriorityNameList()
		, DefaultPriorityIndex(2)
		, AssigneeList()
		, LabelNameList()
	{
		IssueTypeList.Reset();
		IssueTypeList.Add(FText::FromString(TEXT("タスク")));
		IssueTypeList.Add(FText::FromString(TEXT("バグ")));
		IssueTypeList.Add(FText::FromString(TEXT("改善")));

		PriorityNameList.Reset();
		PriorityNameList.Add(FText::FromString(TEXT("Highest")));
		PriorityNameList.Add(FText::FromString(TEXT("High")));
		PriorityNameList.Add(FText::FromString(TEXT("Medium")));
		PriorityNameList.Add(FText::FromString(TEXT("Low")));
		PriorityNameList.Add(FText::FromString(TEXT("Lowest")));
	}


	FString GetAssigneeAccountIdByListIndex(int32 ListIndex) const;
	FText GetAssigneeTextByListIndex(int32 ListIndex) const;
};

/**
 * DebugMenuの文字用のStringTableのリスト
 */
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMStringTableList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UStringTable>> StringTables;

	FGDMStringTableList()
		:StringTables()
	{
	}
};

/**
* リスト表示メニューのカテゴリ名
*/
USTRUCT(BlueprintType)
struct GAMEDEBUGMENU_API FGDMOrderMenuCategoryTitle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Title;

	UPROPERTY(VisibleAnywhere)
	int32 Index;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	FString PreviewTitle;
#endif

	FGDMOrderMenuCategoryTitle()
		: Title()
		, Index(INDEX_NONE)
#if WITH_EDITORONLY_DATA
		, PreviewTitle()
#endif
	{
	}

	FGDMOrderMenuCategoryTitle(FString InTitle)
		: Title(InTitle)
		, Index(INDEX_NONE)
#if WITH_EDITORONLY_DATA
		, PreviewTitle()
#endif
	{
	}

	FGDMOrderMenuCategoryTitle(FString InTitle, int32 InIndex)
		: Title(InTitle)
		, Index(InIndex)
#if WITH_EDITORONLY_DATA
		, PreviewTitle()
#endif
	{
	}

	FGDMOrderMenuCategoryTitle(FString InTitle, FString InPreviewTitle)
		: Title(InTitle)
		, Index(INDEX_NONE)
#if WITH_EDITORONLY_DATA
		, PreviewTitle(InPreviewTitle)
#endif
	{
	}
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGameDebugMenuWidgetInputMappingContextData
{
	GENERATED_BODY()
	
	UPROPERTY(Editanywhere, BlueprintReadWrite)
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(Editanywhere, BlueprintReadWrite)
	int32 Priority = 1000000;
};

/**
 * お気に入り項目の登録情報
 */
USTRUCT(BlueprintType)
struct FGDMFavoriteEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefinitionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveKey;
};