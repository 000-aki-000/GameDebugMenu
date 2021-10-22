/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Framework/Application/NavigationConfig.h"
#include "Launch/Resources/Version.h"
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
};

/**
* プロパティ編集時の最大値、最小値
*/
USTRUCT(BlueprintType)
struct FGDMPropertyRange
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
struct FGDMPropertyUIConfigInfo
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
struct FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EGDMConsoleCommandType Type;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,config)
	EGDMConsoleCommandClickedEvent ClickedEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FText CategoryTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	EGDMConsoleCommandNetType CommandNetType;

	FGDMConsoleCommand()
	{
		CategoryTitle  = FText::FromString(TEXT("Other"));
		Title          = FText::GetEmpty();
		Description    = FText::GetEmpty();
		Type           = EGDMConsoleCommandType::Non;
		ClickedEvent   = EGDMConsoleCommandClickedEvent::Non;
		CommandNetType = EGDMConsoleCommandNetType::LocalOnly;
	}
};

/**
* UIで実行するときに単体で実行する
*/
USTRUCT(BlueprintType)
struct FGDMConsoleCommandSingle : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString ConsoleCommandName;

	FGDMConsoleCommandSingle()
		: Super()
	{
		ConsoleCommandName.Empty();
		Type = EGDMConsoleCommandType::Single;
	}
};

/**
* UIで実行するときに同時に実行できるようにするためのグループ
*/
USTRUCT(BlueprintType)
struct FGDMConsoleCommandGroup : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FString> ConsoleCommandNames;

	FGDMConsoleCommandGroup()
		: Super()
	{
		ConsoleCommandNames.Reset();
		Type = EGDMConsoleCommandType::Group;
	}

};

/**
* UIでトグル操作するときのペアになるコマンド
*/
USTRUCT(BlueprintType)
struct FGDMConsoleCommandPair : public FGDMConsoleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString FirstConsoleCommandName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	FString SecondConsoleCommandName;

	FGDMConsoleCommandPair()
		: Super()
	{
		FirstConsoleCommandName.Empty();
		SecondConsoleCommandName.Empty();
		Type = EGDMConsoleCommandType::Pair;
	}
};

/**
* UIで実行するときに数字指定で実行する
*/
USTRUCT(BlueprintType)
struct FGDMConsoleCommandNumber : public FGDMConsoleCommand
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

	FGDMConsoleCommandNumber()
		: Super()
	{
		ConsoleCommandName.Empty();
		PreConsoleCommandName.Empty();
		PostConsoleCommandName.Empty();
		Type                        = EGDMConsoleCommandType::Number;
		UIConfigInfo.Range.bUseMax  = true;
		UIConfigInfo.Range.bUseMin  = true;
		UIConfigInfo.Range.MinValue = 0.0f;
		UIConfigInfo.Range.MaxValue = 1.0f;
		DefaultValue                = 0.0f;
		ConsoleVariableName.Empty();
	}
};

/**
* DebugMenuの入力イベントのキー
*/
USTRUCT(BlueprintType)
struct FGDMActionMappingKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FKey> Keys;
};

/**
* DebugMenuの入力イベントのキー（Axis版）
*/
USTRUCT(BlueprintType)
struct FGDMAxisMappingKey
{
	GENERATED_BODY()

	/** Scaleと数が必ず一致してること */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<FKey> Keys;

	/** Keysと数が必ず一致してること */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, config)
	TArray<float> Scale;
};

/**
* Menuに登録できるプロパティ情報
*/
class FGDMObjectPropertyInfo
{
public:
	FText Category;
	FText Name;
	FText Description;
	TWeakObjectPtr<UObject>	TargetObject;
	FProperty* TargetProperty;
	FName PropertyName;
	TWeakObjectPtr<UEnum> EnumType;
	FGDMPropertyUIConfigInfo ConfigInfo;


	FGDMObjectPropertyInfo()
		: Category(FText::GetEmpty())
		, Name(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, TargetObject(nullptr)
		, TargetProperty(nullptr)
		, PropertyName(NAME_None)
		, EnumType(nullptr)
		, ConfigInfo()
	{
	}
};

/**
* Menuに登録できる関数情報
*/
class FGDMObjectFunctionInfo
{
public:
	FText Category;
	FText Name;
	FText Description;
	TWeakObjectPtr<UObject>	TargetObject;
	TWeakObjectPtr<UFunction> TargetFunction;
	FName FunctionName;

	FGDMObjectFunctionInfo()
		: Category(FText::GetEmpty())
		, Name(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, TargetObject(nullptr)
		, TargetFunction(nullptr)
		, FunctionName(NAME_None)
	{
	}
};

/**
* 未登録状態のオブジェクト情報
*/
struct FGDMPendingObjectData
{
	TWeakObjectPtr<UObject> TargetObject;
	FName TargetName;
	FText DisplayCategoryName;
	FText DisplayPropertyName;
	FText Description;
	FGDMPropertyUIConfigInfo ConfigInfo;

	FGDMPendingObjectData()
		: TargetObject(nullptr)
		, TargetName(NAME_None)
		, DisplayCategoryName(FText::GetEmpty())
		, DisplayPropertyName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, ConfigInfo()
	{
	}
};

/**
* 
*/
UENUM(BlueprintType)
enum class EGDMProjectManagementTool : uint8
{
	Trello,
	Redmine,
	Jira,
};

USTRUCT(BlueprintType)
struct FGDMProjectManagementToolSettings
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
struct FGDMRedmineSettings : public FGDMProjectManagementToolSettings
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
struct FGDMTrelloSettings : public FGDMProjectManagementToolSettings
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
struct FGDMJiraSettings : public FGDMProjectManagementToolSettings
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


USTRUCT(BlueprintType)
struct FGDMStartupConsoleCommandList
{
	GENERATED_BODY()

	/** コマンド */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Commands;

	FGDMStartupConsoleCommandList()
		:Commands()
	{
	}
};
