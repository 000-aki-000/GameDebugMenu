/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuManager.h"

#include "Engine/DebugCameraController.h"
#include "Kismet/KismetStringLibrary.h"
#include "GameFramework/CheatManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuFunctions.h"
#include "GDMListenerComponent.h"
#include "GDMPlayerControllerProxyComponent.h"
#include "GDMScreenshotRequesterComponent.h"
#include "GDMOutputDevice.h"
#include "Input/GDMInputInterface.h"
#include "Input/GDMInputEventFunctions.h"
#include "Input/GDMDebugCameraInput.h"
#include "Input/GDMInputSystemComponent.h"
#include "Widgets/GameDebugMenuRootWidget.h"
#include "Reports/GDMRequesterRedmine.h"
#include "Reports/GDMRequesterTrello.h"
#include "Reports/GDMRequesterJira.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <Internationalization/StringTableCore.h>
#include "Widgets/GDMTextBlock.h"


/************************************************************************/
/* AGameDebugMenuManager											    */
/************************************************************************/

AGameDebugMenuManager::AGameDebugMenuManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuInputSystemComponent(nullptr)
	, ScreenshotRequesterComponent(nullptr)
	, ListenerComponent(nullptr)
	, bShowDebugMenu(false)
	, bCachedGamePaused(false)
	, bCachedShowMouseCursor(false)
	, bWaitToCaptureBeforeOpeningDebugReportMenu(false)
	, CachedNavigationConfigs()
	, ObjectProperties()
	, ObjectFunctions()
	, DebugMenuRootWidgetClass(nullptr)
	, DebugMenuRootWidget(nullptr)
	, DebugMenuClasses()
	, DebugMenuInstances()
	, WidgetZOrder(999)
	, bGamePause(false)
	, DebugCameraInputClass()
	, DebugCameraInput(nullptr)
	, DebugMenuPCProxyComponentClass()
	, DebugReportRequesterClass()
	, OutputLog(nullptr)
	, DebugMenuStrings()
	, CurrentDebugMenuLanguage()
	, bCurrentDebugMenuDirectStringKey(false)
{
	DebugMenuInputSystemComponent = CreateDefaultSubobject<UGDMInputSystemComponent>(TEXT("DebugMenuInputSystemComponent"));
	ScreenshotRequesterComponent  = CreateDefaultSubobject<UGDMScreenshotRequesterComponent>(TEXT("ScreenshotRequesterComponent"));
	ListenerComponent             = CreateDefaultSubobject<UGDMListenerComponent>(TEXT("ListenerComponent"));

	PrimaryActorTick.bCanEverTick			= true;
	PrimaryActorTick.bStartWithTickEnabled	= true;
	PrimaryActorTick.bTickEvenWhenPaused	= true;
	SetCanBeDamaged(false);
	SetHidden(true);
	InputPriority                           = 999;
	bBlockInput                             = true;
	bReplicates                             = true;
	bAlwaysRelevant                         = true;
	SetReplicatingMovement(false);
	NetUpdateFrequency                      = 1;/* デフォルトのPlayerStateと同じかんじにしとく */
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);

	DebugCameraInputClass          = AGDMDebugCameraInput::StaticClass();
	DebugMenuPCProxyComponentClass = UGDMPlayerControllerProxyComponent::StaticClass();
	DebugReportRequesterClass.Add(EGDMProjectManagementTool::Trello, AGDMRequesterTrello::StaticClass());
	DebugReportRequesterClass.Add(EGDMProjectManagementTool::Redmine,AGDMRequesterRedmine::StaticClass());
	DebugReportRequesterClass.Add(EGDMProjectManagementTool::Jira, AGDMRequesterJira::StaticClass());	
}

void AGameDebugMenuManager::BeginPlay()
{
	Super::BeginPlay();

	UGameDebugMenuFunctions::RegisterGameDebugMenuManagerInstance(this);

	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Call BeginPlay"), 4.0f);

	OutputLog = MakeShareable(new FGDMOutputDevice);

	CurrentDebugMenuLanguage = UGameDebugMenuSettings::Get()->DefaultGameDebugMenuLanguage;

	bCurrentDebugMenuDirectStringKey = UGameDebugMenuSettings::Get()->bGameDebugMenuDirectStringKey;

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		/* managerのBeginplayがちゃんと完了後に処理↓ */

		SyncLoadDebugMenuStringTables(CurrentDebugMenuLanguage);

		CreateDebugCameraInputClass();

		if( !UKismetSystemLibrary::IsDedicatedServer(this) )
		{
			CreateDebugMenuRootWidget();

			if( APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0) )
			{
				const FGDMStartupConsoleCommandList* CommandList = UGameDebugMenuSettings::Get()->StartupConsoleCommands.Find(UGameDebugMenuSettings::Get()->StartupConsoleCommandKeyName);
				if( CommandList != nullptr )
				{
					for( const auto& Command : CommandList->Commands )
					{
						ExecuteConsoleCommand(Command, PC);
					}
				}
				else
				{
					UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Beginplay error StartupConsoleCommands"), 4.0f);
				}
			}
			else
			{
				UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Beginplay Notfound PlayerController"), 4.0f);
			}
		}
	}));
}

void AGameDebugMenuManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Call EndPlay"), 4.0f);

	OutputLog.Reset();

	if(EndPlayReason != EEndPlayReason::EndPlayInEditor && EndPlayReason != EEndPlayReason::Quit)
	{
		EnabledNavigationConfigs();

		DebugMenuInputSystemComponent->CallReleasedButtons();

		if(IsValid(DebugMenuRootWidget))
		{
			DebugMenuRootWidget->RemoveFromParent();
			DebugMenuRootWidget = nullptr;
		}

		if(IsValid(DebugCameraInput))
		{
			DebugCameraInput->K2_DestroyActor();
			DebugCameraInput = nullptr;
		}
	}

	Super::EndPlay(EndPlayReason);

	UGameDebugMenuFunctions::UnregisterGameDebugMenuManagerInstance(this);
}

void AGameDebugMenuManager::Tick(float DeltaTime)
{
	/* Slomoなど時間操作に影響受けないようにする */
	DeltaTime /= GetWorldSettings()->GetEffectiveTimeDilation();

	Super::Tick(DeltaTime);
}

void AGameDebugMenuManager::EnableInput(class APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);

	DebugMenuInputSystemComponent->InitializeInputComponentBindings(InputComponent);

	if(IsValid(PlayerController) && IsValid(PlayerController->CheatManager))
	{
		if(ADebugCameraController* DCC = PlayerController->CheatManager->DebugCameraControllerRef)
		{
			Super::EnableInput(DCC);
		}
		else
		{
			/* この時点でまだない場合,Clientの可能性があるのでローカルで実行できるようにチートを有効にする */
			PlayerController->EnableCheats();
			UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: EnableCheats"), 4.0f);
		}
	}
}

void AGameDebugMenuManager::DisableInput(class APlayerController* PlayerController)
{
	Super::DisableInput(PlayerController);

	if(IsValid(PlayerController) && IsValid(PlayerController->CheatManager))
	{
		if(ADebugCameraController* DCC = PlayerController->CheatManager->DebugCameraControllerRef)
		{
			Super::DisableInput(DCC);
		}
	}
}

UGDMInputSystemComponent* AGameDebugMenuManager::GetDebugMenuInputSystemComponent() const
{
	return DebugMenuInputSystemComponent;
}

UGDMScreenshotRequesterComponent* AGameDebugMenuManager::GetScreenshotRequesterComponent() const
{
	return ScreenshotRequesterComponent;
}

UGDMListenerComponent* AGameDebugMenuManager::GetListenerComponent()
{
	return ListenerComponent;
}

void AGameDebugMenuManager::EnabledNavigationConfigs()
{
	if (CachedNavigationConfigs.Num() > 0)
	{
		/* Navigationを戻す */
		FSlateApplication::Get().SetNavigationConfig(CachedNavigationConfigs[0]);
		CachedNavigationConfigs.Empty();
	}
}

void AGameDebugMenuManager::DisabledNavigationConfigs()
{
	/* Navigationを切る */
	CachedNavigationConfigs.Empty();
	CachedNavigationConfigs.Add(FSlateApplication::Get().GetNavigationConfig());
	FSlateApplication::Get().SetNavigationConfig(MakeShared<FGDMNavigationConfig>());
}

void AGameDebugMenuManager::CreateDebugMenuRootWidget()
{
	if( DebugMenuRootWidgetClass == nullptr)
	{
		return;
	}

	DebugMenuRootWidget = Cast<UGameDebugMenuRootWidget>(UWidgetBlueprintLibrary::Create(this, DebugMenuRootWidgetClass, nullptr));
	DebugMenuRootWidget->DebugMenuManager = this;
	DebugMenuRootWidget->AddToViewport(WidgetZOrder);
	DebugMenuRootWidget->SetVisibility(ESlateVisibility::Collapsed);
	DebugMenuRootWidget->InitializeRootWidget();
}

void AGameDebugMenuManager::CreateDebugCameraInputClass()
{
	if(DebugCameraInputClass == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner                          = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	DebugCameraInput                         = GetWorld()->SpawnActor<AGDMDebugCameraInput>(DebugCameraInputClass, SpawnInfo);

	bool bExistDCC        = false;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if(IsValid(PC) && IsValid(PC->CheatManager))
	{
		if(ADebugCameraController* DCC = PC->CheatManager->DebugCameraControllerRef)
		{
			bExistDCC = true;
			DebugCameraInput->EnableInput(DCC);
		}
	}

	if(!bExistDCC)
	{
		/* ないので生成するのを待つ */
		DebugCameraInput->BindSpawnDebugCameraController();
	}
}

void AGameDebugMenuManager::EnableShowMouseCursorFlag(APlayerController* PlayerController)
{
	bCachedShowMouseCursor = PlayerController->bShowMouseCursor;

	PlayerController->bShowMouseCursor = true;

	if(!IsValid(PlayerController->CheatManager))
	{
		return;
	}

	if(!IsValid(PlayerController->CheatManager->DebugCameraControllerRef))
	{
		return;
	}

	/* DebugCamera操作中Menuを出すとマウスが表示されないのでこっちも合わせて更新 */
	PlayerController->CheatManager->DebugCameraControllerRef->bShowMouseCursor = true;
}

void AGameDebugMenuManager::RestoreShowMouseCursorFlag(APlayerController* PlayerController)
{
	PlayerController->bShowMouseCursor = bCachedShowMouseCursor;

	if(!IsValid(PlayerController->CheatManager))
	{
		return;
	}

	if(!IsValid(PlayerController->CheatManager->DebugCameraControllerRef))
	{
		return;
	}

	/* DebugCameraも合わせて更新 */
	PlayerController->CheatManager->DebugCameraControllerRef->bShowMouseCursor = bCachedShowMouseCursor;
}

void AGameDebugMenuManager::TryEnableGamePause()
{
	if(bGamePause)
	{
		bCachedGamePaused = UGameplayStatics::IsGamePaused(this);
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AGameDebugMenuManager::RestoreGamePause()
{
	if(bGamePause)
	{
		UGameplayStatics::SetGamePaused(this, bCachedGamePaused);
	}
}

void AGameDebugMenuManager::OnScreenshotRequestProcessed()
{
	bWaitToCaptureBeforeOpeningDebugReportMenu = false;

	GetListenerComponent()->OnScreenshotRequestProcessedDispatcher.RemoveDynamic(this, &AGameDebugMenuManager::OnScreenshotRequestProcessed);
	SetIgnoreInput(false);
	GetDebugMenuRootWidget()->ShowDebugReport();
}

void AGameDebugMenuManager::SyncLoadDebugMenuStringTables(FName TargetDebugMenuLanguage)
{
	DebugMenuStrings.Reset();

	if( FGDMStringTableList* StringTableList = UGameDebugMenuSettings::Get()->GameDebugMenuStringTables.Find(TargetDebugMenuLanguage) )
	{
		UE_LOG(LogGDM, Verbose, TEXT("Call SyncLoadDebugMenuStringTables %s"), *TargetDebugMenuLanguage.ToString());

		for( auto& StrTablePtr : StringTableList->StringTables )
		{
			if( UStringTable* StringTable = StrTablePtr.LoadSynchronous() )
			{
				StringTable->GetStringTable()->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
				{
					if( !DebugMenuStrings.Contains(InKey) )
					{
						DebugMenuStrings.Add(InKey, InSourceString);
						UE_LOG(LogGDM, VeryVerbose, TEXT("Load string | Key %s SourceString %s"), *InKey, *InSourceString);
					}
					else
					{
						UE_LOG(LogGDM, Error, TEXT("%s -> StringKey that is already in use!!"));
					}
					return true; /* すべて取得する */
				});
			}
		}
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("Failed Load DebugMenuStringTables %s"), *TargetDebugMenuLanguage.ToString());
	}
}

void AGameDebugMenuManager::ExecuteConsoleCommand(const FString& Command, APlayerController* PC)
{
	if(Command.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		return;
	}

	if(!IsValid(PC))
	{
		return;
	}

	FString LogCommand;
	if(IsValid(PC->CheatManager))
	{
		if(ADebugCameraController* DCC = PC->CheatManager->DebugCameraControllerRef)
		{
			if(!DCC->IsInState(NAME_Inactive))
			{
				/* DebugCameraControllerが動作してた場合はそっちで実行 */
				LogCommand = DCC->ConsoleCommand(Command);
				UGameDebugMenuFunctions::PrintLogScreen(this, FString::Printf(TEXT("ExecuteConsoleCommand DCC: %s: Log %s"), *Command, *LogCommand), 4.0f);
				CallExecuteConsoleCommandDispatcher(Command);
				return;
			}
		}
	}

	LogCommand = PC->ConsoleCommand(Command);
	UGameDebugMenuFunctions::PrintLogScreen(this, FString::Printf(TEXT("ExecuteConsoleCommand PC: %s: Log %s"), *Command, *LogCommand), 4.0f);
	CallExecuteConsoleCommandDispatcher(Command);
}

bool AGameDebugMenuManager::ShowDebugMenu(bool bWaitToCaptureBeforeOpeningMenuFlag)
{
	if(bShowDebugMenu)
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("ShowDebugMenu: bShowDebugMenu TRUE"), 4.0f);
		return false;
	}

	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("ShowDebugMenu: Not found World"), 4.0f);
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if(!IsValid(PC))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("ShowDebugMenu: Not found PlayerController"), 4.0f);
		return false;
	}

	if(!IsValid(DebugMenuRootWidget))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("ShowDebugMenu: Not found DebugMenuRootWidget"), 4.0f);
		return false;
	}

	bShowDebugMenu = true;
	EnableInput(PC);
	DisabledNavigationConfigs();
	EnableShowMouseCursorFlag(PC);
	TryEnableGamePause();

	GetScreenshotRequesterComponent()->RequestScreenshot();

	if(bWaitToCaptureBeforeOpeningMenuFlag)
	{
		bWaitToCaptureBeforeOpeningDebugReportMenu = true;

		/* 画面キャプチャ後UIを開くまで入力を無視するように */
		SetIgnoreInput(true);
		GetListenerComponent()->OnScreenshotRequestProcessedDispatcher.AddDynamic(this, &AGameDebugMenuManager::OnScreenshotRequestProcessed);
	}
	else
	{
		TArray<UGameDebugMenuWidget*> DebugMenuWidgets = AGameDebugMenuManager::GetViewportDebugMenuWidgets();
		for ( auto ViewportWidget : DebugMenuWidgets )
		{
			if( ViewportWidget->IsActivateDebugMenu() )
			{
				ViewportWidget->OnShowingMenu();
			}
		}
	}

	CallShowDispatcher();

	UE_LOG(LogGDM, Log, TEXT("ShowDebugMenu"));
	return true;
}

void AGameDebugMenuManager::HideDebugMenu()
{
	if(!bShowDebugMenu)
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: bShowDebugMenu FALSE"), 4.0f);
		return;
	}

	if(bWaitToCaptureBeforeOpeningDebugReportMenu)
	{
		/* キャプチャ後１度UIを開く予定なので閉じないようにする */
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: bWaitToCaptureBeforeOpeningMenu TRUE"), 4.0f);
		return;
	}

	UWorld* World = GetWorld();
	if(!IsValid(World))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: Not found World"), 4.0f);
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World,0);
	if(!IsValid(PC))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: Not found PlayerController"), 4.0f);
		return;
	}

	if(!IsValid(DebugMenuRootWidget))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: Not found DebugMenuWidget"), 4.0f);
		return;
	}

	bShowDebugMenu = false;

	DisableInput(PC);
	EnabledNavigationConfigs();
	RestoreShowMouseCursorFlag(PC);
	RestoreGamePause();

	DebugMenuInputSystemComponent->CallReleasedButtons();

	TArray<UGameDebugMenuWidget*> DebugMenuWidgets = AGameDebugMenuManager::GetViewportDebugMenuWidgets();
	for( auto ViewportWidget : DebugMenuWidgets )
	{
		if( ViewportWidget->IsActivateDebugMenu() )
		{
			ViewportWidget->OnHidingMenu();
		}
	}

	CallHideDispatcher();

	UE_LOG(LogGDM, Log, TEXT("HideDebugMenu"));
}

bool AGameDebugMenuManager::IsShowingDebugMenu()
{
	return bShowDebugMenu;
}

UGameDebugMenuRootWidget* AGameDebugMenuManager::GetDebugMenuRootWidget()
{
	return DebugMenuRootWidget;
}

TSubclassOf<AGDMDebugReportRequester>* AGameDebugMenuManager::GetDebugReportRequesterClass()
{
	EGDMProjectManagementTool ProjectManagementTool = UGameDebugMenuSettings::Get()->ProjectManagementToolType;
	return DebugReportRequesterClass.Find(ProjectManagementTool);
}

int32 AGameDebugMenuManager::GetAllDebugMenuKeys(TArray<FString>& OutKeys)
{
	OutKeys = DebugMenuRegistrationOrder;
	return OutKeys.Num();
}

TSubclassOf<UGameDebugMenuWidget> AGameDebugMenuManager::GetDebugMenuWidgetClass(const FString& Key)
{
	TSubclassOf<UGameDebugMenuWidget>* WidgetClass = DebugMenuClasses.Find(Key);
	if( WidgetClass == nullptr )
	{
		return nullptr;
	}
	return (*WidgetClass);
}

bool AGameDebugMenuManager::GetDebugMenuWidgetInstances(TArray<UGameDebugMenuWidget*>& OutInstances)
{
	DebugMenuInstances.GenerateValueArray(OutInstances);
	return (OutInstances.Num() > 0);
}

void AGameDebugMenuManager::GetOutputLogStrings(TArray<FString>& OutLogs)
{
	OutLogs = OutputLog->Logs;
}

void AGameDebugMenuManager::GetOutputLogString(FString& OutLog, const FString& Separator)
{
	OutLog = UKismetStringLibrary::JoinStringArray(OutputLog->Logs, Separator);
}

EGDMPropertyType AGameDebugMenuManager::GetPropertyType(FProperty* TargetProperty)
{
	if(TargetProperty != nullptr)
	{
		if(TargetProperty->IsA(FBoolProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_Bool;
		}
		else if(TargetProperty->IsA(FIntProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_Int;
		}
		else if(TargetProperty->IsA(FFloatProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_Float;
		}
		else if(TargetProperty->IsA(FEnumProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_Enum;
		}
		else if(TargetProperty->IsA(FByteProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_Byte;
		}
		else if(TargetProperty->IsA(FStrProperty::StaticClass()))
		{
			return EGDMPropertyType::GDM_String;
		}
	}

	return EGDMPropertyType::GDM_Null;
}

bool AGameDebugMenuManager::RegisterObjectProperty(UObject* TargetObject,FName PropertyName, const FGDMGameplayCategoryKey& CategoryKey,const FText& DisplayPropertyName,const FText& Description, const FGDMPropertyUIConfigInfo& PropertyUIConfigInfo, const int32& DisplayPriority)
{
	if(!IsValid(TargetObject))
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectProperty: Not found TargetObject"));
		return false;
	}

	FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
	if( Property == nullptr )
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectProperty: Not found Property"));
		return false;
	}

	if(GetPropertyType(Property) == EGDMPropertyType::GDM_Null)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectProperty: not supported Property: %s: %s"), *PropertyName.ToString(), *GetNameSafe(Property));
		return false;
	}

	TSharedPtr<FGDMObjectPropertyInfo> PropertyInfo = MakeShareable(new FGDMObjectPropertyInfo);
	PropertyInfo->CategoryKey                       = CategoryKey;
	PropertyInfo->Name                              = (DisplayPropertyName.IsEmpty() != false) ? FText::FromName(PropertyName) : DisplayPropertyName;
	PropertyInfo->TargetObject                      = TargetObject;
	PropertyInfo->PropertyName                      = PropertyName;
	PropertyInfo->TargetProperty                    = Property;
	PropertyInfo->ConfigInfo                        = PropertyUIConfigInfo;
	PropertyInfo->Description						= Description;
	PropertyInfo->DisplayPriority					= DisplayPriority;

	/* Enumならセット */
	const FEnumProperty* EnumProp = CastField<FEnumProperty>(Property);/* C++定義だとこっち */
	const FByteProperty* ByteProp = CastField<FByteProperty>(Property);/* BP定義だとこっちみたい… */

	if(EnumProp != nullptr)
	{
		PropertyInfo->EnumType = EnumProp->GetEnum();
	}
	else if(ByteProp != nullptr)
	{
		PropertyInfo->EnumType = ByteProp->Enum;
	}

	ObjectProperties.Add(PropertyInfo);

	ObjectProperties.Sort([](const TSharedPtr<FGDMObjectPropertyInfo>& A,const TSharedPtr<FGDMObjectPropertyInfo>& B)
	{
		return A->DisplayPriority >= B->DisplayPriority;
	});

	return true;
}

bool AGameDebugMenuManager::RegisterObjectFunction(UObject* TargetObject,FName FunctionName, const FGDMGameplayCategoryKey& CategoryKey,const FText& DisplayFunctionName,const FText& Description,const int32& DisplayPriority)
{
	if(!IsValid(TargetObject))
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectFunction: Not found TargetObject"));
		return false;
	}

	UFunction* Function = TargetObject->GetClass()->FindFunctionByName(FunctionName);
	if( Function == nullptr )
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectFunction: Not found Function"));
		return false;
	}

	if (Function->NumParms != 0)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectFunction: Arguments are not supported: %d"), Function->NumParms);
		return false;
	}

	TSharedPtr<FGDMObjectFunctionInfo> FunctionInfo = MakeShareable(new FGDMObjectFunctionInfo);
	FunctionInfo->CategoryKey                       = CategoryKey;
	FunctionInfo->Name                              = (DisplayFunctionName.IsEmpty() != false) ? FText::FromName(FunctionName) : DisplayFunctionName;
	FunctionInfo->TargetObject                      = TargetObject;
	FunctionInfo->FunctionName                      = FunctionName;
	FunctionInfo->TargetFunction                    = Function;
	FunctionInfo->Description						= Description;
	FunctionInfo->DisplayPriority					= DisplayPriority;

	ObjectFunctions.Add(FunctionInfo);

	ObjectFunctions.Sort([](const TSharedPtr<FGDMObjectFunctionInfo>& A,const TSharedPtr<FGDMObjectFunctionInfo>& B)
	{
		return A->DisplayPriority >= B->DisplayPriority;
	});

	return true;
}

UObject* AGameDebugMenuManager::GetObjectProperty(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FName& OutEnumTypeName, FGDMPropertyUIConfigInfo& OutPropertyUIConfigInfo)
{
	OutPropertyType = EGDMPropertyType::GDM_Null;

	if(!ObjectProperties.IsValidIndex(Index))
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectProperty: Not found Index: %d"), Index);
		return nullptr;
	}

	const auto& ObjProp = ObjectProperties[Index];
	if(!ObjProp->TargetObject.IsValid())
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectProperty: Not found TargetObject"));
		return nullptr;
	}

	const auto Property = ObjProp->TargetProperty;
	if(Property == nullptr)
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectProperty: Not found TargetProperty"));
		return nullptr;
	}

	OutCategoryKey			= ObjProp->CategoryKey;
	OutDisplayPropertyName	= ObjProp->Name;
	OutDescription			= ObjProp->Description;
	OutPropertyName			= ObjProp->PropertyName;
	OutPropertyUIConfigInfo = ObjProp->ConfigInfo;
	OutPropertyType			= GetPropertyType(Property);

	if(OutPropertyType == EGDMPropertyType::GDM_Enum)
	{
		OutEnumTypeName = ObjProp->EnumType->GetFName();
	}
	else if(OutPropertyType == EGDMPropertyType::GDM_Byte)
	{
		if(ObjProp->EnumType.IsValid())
		{
			/* Enumならセット */
			OutPropertyType = EGDMPropertyType::GDM_Enum;
			OutEnumTypeName = ObjProp->EnumType->GetFName();
		}
	}

	return ObjProp->TargetObject.Get();
}

void AGameDebugMenuManager::RemoveObjectProperty(const int32 Index)
{
	ObjectProperties.RemoveAt(Index);
}

UObject* AGameDebugMenuManager::GetObjectFunction(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName)
{
	if(!ObjectFunctions.IsValidIndex(Index))
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectFunction: Not found Index: %d"),Index);
		return nullptr;
	}

	const auto& ObjFunc = ObjectFunctions[Index];
	if(!ObjFunc->TargetObject.IsValid())
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectFunction: Not found TargetObject"));
		return nullptr;
	}

	const auto& Function = ObjFunc->TargetFunction;
	if(!Function.IsValid())
	{
		UE_LOG(LogGDM, Warning, TEXT("GetObjectFunction: Not found TargetFunction"));
		return nullptr;
	}

	OutCategoryKey		   = ObjFunc->CategoryKey;
	OutDisplayFunctionName = ObjFunc->Name;
	OutDescription         = ObjFunc->Description;
	OutFunctionName		   = ObjFunc->FunctionName;

	return ObjFunc->TargetObject.Get();
}

void AGameDebugMenuManager::RemoveObjectFunction(const int32 Index)
{
	ObjectFunctions.RemoveAt(Index);
}

int32 AGameDebugMenuManager::GetNumObjectProperties()
{
	return ObjectProperties.Num();
}

int32 AGameDebugMenuManager::GetNumObjectFunctions()
{
	return ObjectFunctions.Num();
}

void AGameDebugMenuManager::AddDebugMenuPCProxyComponent(APlayerController* PlayerController)
{
	if (!IsValid(DebugMenuPCProxyComponentClass))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddDebugMenuPCProxyComponent: Not found DebugMenuPCProxyComponentClass"));
		return;
	}

	UActorComponent* NewComponent = NewObject<UActorComponent>(PlayerController, DebugMenuPCProxyComponentClass);
	if (!IsValid(NewComponent))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddDebugMenuPCProxyComponent; Not found NewComponent"));
		return;
	}

	/* 登録 */
	NewComponent->RegisterComponent();
}

bool AGameDebugMenuManager::RegisterInputObject(UObject* TargetObject)
{
	if( DebugMenuInputSystemComponent->RegisterInputObject(TargetObject) )
	{
		CallRegisterInputSystemEventDispatcher(TargetObject);
		return true;
	}

	return false;
}

bool AGameDebugMenuManager::UnregisterInputObject(UObject* TargetObject)
{
	if( DebugMenuInputSystemComponent->UnregisterInputObject(TargetObject) )
	{
		CallUnregisterInputSystemEventDispatcher(TargetObject);
		return true;
	}

	return false;
}

void AGameDebugMenuManager::SetIgnoreInput(bool bNewInput)
{
	DebugMenuInputSystemComponent->SetIgnoreInput(bNewInput);
}

void AGameDebugMenuManager::ResetIgnoreInput()
{
	DebugMenuInputSystemComponent->ResetIgnoreInput();
}

bool AGameDebugMenuManager::IsInputIgnored() const
{
	return DebugMenuInputSystemComponent->IsInputIgnored();
}

void AGameDebugMenuManager::ChangeDebugMenuLanguage(FName LanguageKey, bool bForcedUpdate)
{
	UE_LOG(LogGDM, Verbose, TEXT("Call ChangeDebugMenuLanguage LanguageKey:%s bForcedUpdate:%d"), *LanguageKey.ToString(), bForcedUpdate);

	if( !bForcedUpdate )
	{
		if( LanguageKey == NAME_None )
		{
			return;
		}

		if( CurrentDebugMenuLanguage == LanguageKey )
		{
			return;
		}
	}

	FName Old = CurrentDebugMenuLanguage;
	CurrentDebugMenuLanguage = LanguageKey;

	SyncLoadDebugMenuStringTables(CurrentDebugMenuLanguage);

	TArray<UWidget*> ChildWidgets;
	for( auto ViewportWidget : ViewportDebugMenuWidgets )
	{
		/* Viewportに追加 Widget 内にあるすべてのTextBlockとDebugMenuWidgetを更新 */
		{
			ViewportWidget->GetWidgetChildrenOfClass(UGDMTextBlock::StaticClass(), ChildWidgets, false);

			for( auto ChildWidget : ChildWidgets )
			{
				if( UGDMTextBlock* TextBlock = Cast<UGDMTextBlock>(ChildWidget) )
				{
					if( !TextBlock->DebugMenuStringKey.IsEmpty() )
					{
						TextBlock->SetText(FText::FromString(TextBlock->DebugMenuStringKey));
					}
				}
			}

			ViewportWidget->GetWidgetChildrenOfClass(UGameDebugMenuWidget::StaticClass(), ChildWidgets, false);

			for( auto ChildWidget : ChildWidgets )
			{
				if( UGameDebugMenuWidget* DebugMenuWidget = Cast<UGameDebugMenuWidget>(ChildWidget) )
				{
					DebugMenuWidget->OnChangeDebugMenuLanguage(LanguageKey, Old);
				}
			}
		}

		ViewportWidget->OnChangeDebugMenuLanguage(LanguageKey, Old);
	}

	CallChangeDebugMenuLanguageDispatcher(LanguageKey, Old);
}

bool AGameDebugMenuManager::GetDebugMenuString(const FString& StringKey, FString& OutString)
{
	if( FString* SourceString = DebugMenuStrings.Find(StringKey) )
	{
		if( bCurrentDebugMenuDirectStringKey )
		{
			/* 取得できたキーをそのまま戻す */
			OutString = StringKey;
		}
		else
		{
			OutString = *SourceString;
		}
	}
	else
	{
		OutString = StringKey;
		UE_LOG(LogGDM, Verbose, TEXT("GetDebugMenuString: Not found StringKey %s"), *StringKey);
		return false;
	}

	return true;
}

TArray<FName> AGameDebugMenuManager::GetDebugMenuLanguageKeys()
{
	TArray<FName> ReturnValues;
	UGameDebugMenuSettings::Get()->GameDebugMenuStringTables.GetKeys(ReturnValues);
	return ReturnValues;
}

void AGameDebugMenuManager::RegisterViewportDebugMenuWidget(UGameDebugMenuWidget* TargetWidget)
{
	ViewportDebugMenuWidgets.AddUnique(TargetWidget);
}

TArray<UGameDebugMenuWidget*> AGameDebugMenuManager::GetViewportDebugMenuWidgets()
{
	return ViewportDebugMenuWidgets;
}

void AGameDebugMenuManager::UnregisterViewportDebugMenuWidget(UGameDebugMenuWidget* TargetWidget)
{
	ViewportDebugMenuWidgets.RemoveSingle(TargetWidget);
}

void AGameDebugMenuManager::CallExecuteConsoleCommandDispatcher(const FString& Command)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnExecuteConsoleCommandDispatcher.Broadcast(Command);
	}
}

void AGameDebugMenuManager::CallShowDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnShowDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallHideDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnHideDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallRegisterInputSystemEventDispatcher(UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnRegisterInputSystemDispatcher.Broadcast(TargetObject);
	}
}

void AGameDebugMenuManager::CallUnregisterInputSystemEventDispatcher(UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnUnregisterInputSystemDispatcher.Broadcast(TargetObject);
	}
}

void AGameDebugMenuManager::CallExecuteProcessEventDispatcher(const FName& FunctionName, UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnExecuteProcessEventDispatcher.Broadcast(FunctionName, TargetObject);
	}
}

void AGameDebugMenuManager::CallChangePropertyBoolDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnChangePropertyBoolDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old);
	}
}

void AGameDebugMenuManager::CallChangePropertyIntDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnChangePropertyIntDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old);
	}
}

void AGameDebugMenuManager::CallChangePropertyFloatDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnChangePropertyFloatDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old);
	}
}

void AGameDebugMenuManager::CallChangePropertyByteDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnChangePropertyByteDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old);
	}
}

void AGameDebugMenuManager::CallChangePropertyStringDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(auto& Compoent : ListenerComponents)
	{
		Compoent->OnChangePropertyStringDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old);
	}
}

void AGameDebugMenuManager::CallChangeActiveInputObjectDispatcher(UObject* NewTargetObject, UObject* OldTargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnChangeActiveInputObjectDispatcher.Broadcast(NewTargetObject, OldTargetObject);
	}
}

void AGameDebugMenuManager::CallChangeDebugMenuLanguageDispatcher(const FName& NewLanguageKey, const FName& OldLanguageKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnChangeDebugMenuLanguageDispatcher.Broadcast(NewLanguageKey, OldLanguageKey);
	}
}

void AGameDebugMenuManager::CallStartScreenshotRequestDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnStartScreenshotRequestDispatcher.Broadcast();
	}

}

void AGameDebugMenuManager::CallScreenshotRequestProcessedDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for( auto& Compoent : ListenerComponents )
	{
		Compoent->OnScreenshotRequestProcessedDispatcher.Broadcast();
	}

}
