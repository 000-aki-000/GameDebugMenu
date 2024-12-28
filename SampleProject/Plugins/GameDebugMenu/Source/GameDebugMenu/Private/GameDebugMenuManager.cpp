/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuManager.h"

#include "Engine/DebugCameraController.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include <Internationalization/StringTableCore.h>
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuFunctions.h"
#include "Component/GDMListenerComponent.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Component/GDMScreenshotRequesterComponent.h"
#include "Log/GDMOutputDevice.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Component/GDMPropertyJsonSystemComponent.h"
#include "Component/GDMSaveSystemComponent.h"
#include "Input/GDMDebugCameraInput.h"
#include "Input/GDMInputSystemComponent.h"
#include "Widgets/GameDebugMenuRootWidget.h"
#include "Widgets/GDMTextBlock.h"
#include "Data/GameDebugMenuWidgetDataAsset.h"
#include "Framework/Application/SlateApplication.h"

/********************************************************************/
/* AGameDebugMenuManager										*/
/********************************************************************/

AGameDebugMenuManager::AGameDebugMenuManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuInputSystemComponent(nullptr)
	, ScreenshotRequesterComponent(nullptr)
	, PropertyJsonSystemComponent(nullptr)
	, SaveSystemComponent(nullptr)
	, ListenerComponent(nullptr)
	, bShowDebugMenu(false)
	, bCachedGamePaused(false)
	, bCachedShowMouseCursor(false)
	, bWaitToCaptureBeforeOpeningDebugReportMenu(false)
	, CachedNavigationConfigs()
	, ObjectProperties()
	, ObjectFunctions()
	, DebugMenuRootWidget(nullptr)
	, DebugMenuInstances()
	, bGamePause(false)
	, DebugCameraInputClass()
	, DebugCameraInput(nullptr)
	, DebugMenuPCProxyComponentClass()
	, OutputLog(nullptr)
	, DebugMenuStrings()
{
	DebugMenuInputSystemComponent = CreateDefaultSubobject<UGDMInputSystemComponent>(TEXT("DebugMenuInputSystemComponent"));
	ScreenshotRequesterComponent  = CreateDefaultSubobject<UGDMScreenshotRequesterComponent>(TEXT("ScreenshotRequesterComponent"));
	PropertyJsonSystemComponent   = CreateDefaultSubobject<UGDMPropertyJsonSystemComponent>(TEXT("PropertyJsonSystemComponent"));
	SaveSystemComponent			  = CreateDefaultSubobject<UGDMSaveSystemComponent>(TEXT("SaveSystemComponent"));
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
	SetNetUpdateFrequency(1);/* デフォルトのPlayerStateと同じかんじにしとく */
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);

	DebugCameraInputClass          = AGDMDebugCameraInput::StaticClass();
	DebugMenuPCProxyComponentClass = UGDMPlayerControllerProxyComponent::StaticClass();
}

void AGameDebugMenuManager::BeginPlay()
{
	OutputLog = MakeShareable(new FGDMOutputDevice);
	
	/* 他で参照される前にロードは処理しとく */
	GetSaveSystemComponent()->LoadDebugMenuFile();
	
	Super::BeginPlay();

	UGameDebugMenuFunctions::RegisterGameDebugMenuManagerInstance(this);
	
	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Call BeginPlay"), 4.0f);

	/* managerのBeginplayがちゃんと完了後に処理↓ */
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		OnInitializeManager();
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

UGDMPropertyJsonSystemComponent* AGameDebugMenuManager::GetPropertyJsonSystemComponent() const
{
	return PropertyJsonSystemComponent;
}

UGDMSaveSystemComponent* AGameDebugMenuManager::GetSaveSystemComponent() const
{
	return SaveSystemComponent;
}

UGDMListenerComponent* AGameDebugMenuManager::GetListenerComponent() const
{
	return ListenerComponent;
}

void AGameDebugMenuManager::OnInitializeManager()
{
	FString StringKey = TEXT("DebugMenuDirectStringKey");
	if (!GetPropertyJsonSystemComponent()->HasStringInJson(StringKey))
	{
		GetPropertyJsonSystemComponent()->SetSingleStringToJson(StringKey, TEXT("False"));
		bCurrentDebugMenuDirectStringKey = false;
	}
	else
	{
		bCurrentDebugMenuDirectStringKey = GetPropertyJsonSystemComponent()->GetSingleStringFromJson(StringKey, TEXT("False")).ToBool();
	}

	StringKey = TEXT("DebugMenuLanguage");
	if (!GetPropertyJsonSystemComponent()->HasStringInJson(StringKey))
	{
		GetPropertyJsonSystemComponent()->SetSingleStringToJson(StringKey, UGameDebugMenuSettings::Get()->DefaultGameDebugMenuLanguage.ToString());
	}
	
	SyncLoadDebugMenuStringTables(GetCurrentDebugMenuLanguage());
	
	if( !UKismetSystemLibrary::IsDedicatedServer(this) )
	{
		CreateDebugCameraInputClass();
		
		CreateDebugMenuRootWidget();

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UGameDebugMenuWidget::StaticClass(), false);
						
		UGameViewportSubsystem* GameViewportSubsystem = UGameViewportSubsystem::Get();
		GameViewportSubsystem->OnWidgetAdded.AddUObject(this, &AGameDebugMenuManager::OnWidgetAdded);
		GameViewportSubsystem->OnWidgetRemoved.AddUObject(this, &AGameDebugMenuManager::OnWidgetRemoved);

		for(const auto W : FoundWidgets)
		{
			if(UGameDebugMenuWidget* DebugMenuWidget = Cast<UGameDebugMenuWidget>(W))
			{
				ViewportDebugMenuWidgets.AddUnique(DebugMenuWidget);
			}
		}
	}

	if( APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0) )
	{
		TArray<FString> CommandHistory = GetPropertyJsonSystemComponent()->GetStringArrayFromJson(TEXT("CommandHistory"));
		for(const auto& Command : CommandHistory )
		{
			PC->ConsoleCommand(Command);
		}
	}
	else
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Beginplay Notfound PlayerController"), 4.0f);
	}
}

void AGameDebugMenuManager::CreateDebugMenuRootWidget()
{
	if( !IsValid(WidgetDataAsset) )
	{
		UE_LOG(LogGDM, Warning, TEXT("CreateDebugMenuRootWidget: Not found WidgetDataAsset"));
		return;
	}

	if( IsValid(DebugMenuRootWidget) )
	{
		DebugMenuRootWidget->RemoveFromParent();
	}

	DebugMenuRootWidget = Cast<UGameDebugMenuRootWidget>(UWidgetBlueprintLibrary::Create(this, WidgetDataAsset->DebugMenuRootWidgetClass, nullptr));
	DebugMenuRootWidget->DebugMenuManager = this;
	DebugMenuRootWidget->AddToViewport(WidgetDataAsset->WidgetZOrder);
	DebugMenuRootWidget->SetVisibility(ESlateVisibility::Collapsed);
	DebugMenuRootWidget->InitializeRootWidget();
}

void AGameDebugMenuManager::EnabledNavigationConfigs()
{
	if( CachedNavigationConfigs.Num() > 0 )
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
	const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
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

void AGameDebugMenuManager::RestoreShowMouseCursorFlag(APlayerController* PlayerController) const
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

void AGameDebugMenuManager::RestoreGamePause() const
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
			if(const UStringTable* StringTable = StrTablePtr.LoadSynchronous() )
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
						UE_LOG(LogGDM, Error, TEXT("%s -> StringKey that is already in use!!"), *InKey);
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

	const UWorld* World = GetWorld();
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

	const UWorld* World = GetWorld();
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

	bool bShow = true;

	if( !UGameDebugMenuSettings::Get()->bDisableScreenCaptureProcessingWhenOpeningDebugMenu )
	{
		GetScreenshotRequesterComponent()->RequestScreenshot();

		if( bWaitToCaptureBeforeOpeningMenuFlag )
		{
			/* 画面キャプチャ後UIを出す必要がある */

			/* このフレームではメニューを開かない */
			bShow = false;

			/* キャプチャ後メニューを開けるようにするための識別フラグをたてる */
			bWaitToCaptureBeforeOpeningDebugReportMenu = true;

			/* 開くまで入力を無視するようにする */
			SetIgnoreInput(true);

			GetListenerComponent()->OnScreenshotRequestProcessedDispatcher.AddDynamic(this, &AGameDebugMenuManager::OnScreenshotRequestProcessed);
		}
	}

	if( bShow )
	{
		TArray<UGameDebugMenuWidget*> DebugMenuWidgets = GetViewportDebugMenuWidgets();
		for(const auto ViewportWidget : DebugMenuWidgets )
		{
			if( ViewportWidget->IsActivateDebugMenu() )
			{
				ViewportWidget->OnShowingMenu(/* bRequestDebugMenuManager */true);
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

	const UWorld* World = GetWorld();
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
	GetSaveSystemComponent()->SaveDebugMenuFile();

	DebugMenuInputSystemComponent->CallReleasedButtons();

	TArray<UGameDebugMenuWidget*> DebugMenuWidgets = GetViewportDebugMenuWidgets();
	for(const auto ViewportWidget : DebugMenuWidgets )
	{
		if( ViewportWidget->IsActivateDebugMenu() )
		{
			ViewportWidget->OnHidingMenu(/* bRequestDebugMenuManager */true);
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

int32 AGameDebugMenuManager::GetAllDebugMenuKeys(TArray<FString>& OutKeys)
{
	if( IsValid(WidgetDataAsset) )
	{
		OutKeys = WidgetDataAsset->DebugMenuRegistrationOrder;
	}
	else
	{
		UE_LOG(LogGDM, Warning, TEXT("GetAllDebugMenuKeys: Not found WidgetDataAsset"));
	}

	return OutKeys.Num();
}

TSubclassOf<UGameDebugMenuWidget> AGameDebugMenuManager::GetDebugMenuWidgetClass(const FString& Key)
{
	if( IsValid(WidgetDataAsset) )
	{
		if(const TSubclassOf<UGameDebugMenuWidget>* WidgetClass = WidgetDataAsset->DebugMenuClasses.Find(Key) )
		{
			return (*WidgetClass);
		}
	}
	else
	{
		UE_LOG(LogGDM, Warning, TEXT("GetAllDebugMenuKeys: Not found WidgetDataAsset"));
	}

	return nullptr;
}

bool AGameDebugMenuManager::GetDebugMenuWidgetInstances(TArray<UGameDebugMenuWidget*>& OutInstances)
{
	TArray<TObjectPtr<UGameDebugMenuWidget>> Array; 
	DebugMenuInstances.GenerateValueArray(Array);

	OutInstances.Reset();
	OutInstances.Reserve(Array.Num());

	for (auto A : Array)
	{
		OutInstances.Add(A);
	}
	
	return (OutInstances.Num() > 0);
}

void AGameDebugMenuManager::GetOutputLogString(FString& OutLog, const FString& Separator)
{
	OutLog = UKismetStringLibrary::JoinStringArray(OutputLog->GetLogs(), Separator);
}

void AGameDebugMenuManager::GetOutputCommandHistoryString(TArray<FString>& OutCommandHistory)
{
	OutCommandHistory = OutputLog->GetCommandHistory();
}

EGDMPropertyType AGameDebugMenuManager::GetPropertyType(FProperty* TargetProperty)
{
	if(TargetProperty != nullptr)
	{
		/* メニューで対応できるプロパティかチェック */

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
		else if(TargetProperty->IsA(FDoubleProperty::StaticClass()))
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
		else if( TargetProperty->IsA(FStructProperty::StaticClass()) )
		{
			const FStructProperty* StructProp = CastFieldChecked<FStructProperty>(TargetProperty);
			if( StructProp->Struct->GetFName() == NAME_Vector2D )
			{
				return EGDMPropertyType::GDM_Vector2D;
			}
			else if( StructProp->Struct->GetFName() == NAME_Vector )
			{
				return EGDMPropertyType::GDM_Vector;
			}
			else if( StructProp->Struct->GetFName() == NAME_Rotator )
			{
				return EGDMPropertyType::GDM_Rotator;
			}
		}
	}

	return EGDMPropertyType::GDM_Null;
}

#define SET_VARIANT_PROPERTY(PropertyClass) \
{ \
	OldValueVariant = CastField<PropertyClass>(Property)->GetPropertyValue(ValuePtr); \
}

#define SET_VARIANT_STRUCT_PROPERTY(Class) \
{ \
	const FStructProperty* StructProperty = CastField<FStructProperty>(Property); \
	OldValueVariant = *StructProperty->ContainerPtrToValuePtr<Class>(TargetObject); \
}

#define SET_VARIANT_ENUM_PROPERTY() \
{ \
	const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(Property); \
	const FNumericProperty* NumProp = EnumProp->GetUnderlyingProperty(); \
	const uint64 Value = NumProp->GetUnsignedIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void*>(TargetObject)); \
	OldValueVariant = static_cast<uint8>( Value ); \
}

#define CALL_PROPERTY_DISPATCHER(Type, PropertyClass, ValueType) \
{ \
	const PropertyClass* TypedProperty = CastField<PropertyClass>(Property); \
	ValueType CurrentValue = TypedProperty->GetPropertyValue(ValuePtr); \
	ValueType OldValue = OldValueVariant.GetValue<ValueType>(); \
	if (CurrentValue != OldValue) \
	{ \
		CallChangeProperty##Type##Dispatcher(PropertyName, TargetObject, CurrentValue, OldValue, PropertySaveKey); \
	} \
}

#define CALL_STRUCT_PROPERTY_DISPATCHER(Type, StructType) \
{ \
	const FStructProperty* StructProperty = CastField<FStructProperty>(Property); \
	StructType CurrentValue = *StructProperty->ContainerPtrToValuePtr<StructType>(TargetObject); \
	StructType OldValue = OldValueVariant.GetValue<StructType>(); \
	if (CurrentValue != OldValue) \
	{ \
		CallChangeProperty##Type##Dispatcher(PropertyName, TargetObject, CurrentValue, OldValue, PropertySaveKey); \
	} \
}

#define CALL_ENUM_PROPERTY_DISPATCHER() \
{ \
	const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property); \
	const FNumericProperty* NumProp = EnumProperty->GetUnderlyingProperty(); \
	uint8 CurrentValue = static_cast<uint8>(NumProp->GetUnsignedIntPropertyValue(ValuePtr)); \
	uint8 OldValue = OldValueVariant.GetValue<uint8>(); \
	if (CurrentValue != OldValue) \
	{ \
		CallChangePropertyByteDispatcher(PropertyName, TargetObject, CurrentValue, OldValue, PropertySaveKey); \
	} \
}

bool AGameDebugMenuManager::RegisterObjectProperty(UObject* TargetObject, const FName PropertyName, const FGDMGameplayCategoryKey& CategoryKey, const FString& PropertySaveKey, const FText& DisplayPropertyName, const FText& Description, const FGDMPropertyUIConfigInfo& PropertyUIConfigInfo, const int32& DisplayPriority)
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

	const EGDMPropertyType PropertyType = GetPropertyType(Property);
	if(PropertyType == EGDMPropertyType::GDM_Null)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterObjectProperty: not supported Property: %s: %s"), *PropertyName.ToString(), *GetNameSafe(Property));
		return false;
	}

	const TSharedPtr<FGDMObjectPropertyInfo> PropertyInfo = MakeShareable(new FGDMObjectPropertyInfo);
	PropertyInfo->CategoryKey = CategoryKey;
	PropertyInfo->Name = (DisplayPropertyName.IsEmpty() != false) ? FText::FromName(PropertyName) : DisplayPropertyName;
	PropertyInfo->TargetObject = TargetObject;
	PropertyInfo->PropertyName = PropertyName;
	PropertyInfo->TargetProperty = Property;
	PropertyInfo->ConfigInfo = PropertyUIConfigInfo;
	PropertyInfo->Description = Description;
	PropertyInfo->DisplayPriority = DisplayPriority;
	PropertyInfo->PropertySaveKey = PropertySaveKey;

	/* Enumならセット */
	const FEnumProperty* EnumProp = CastField<FEnumProperty>(Property);/* C++定義だとこっち */
	const FByteProperty* ByteProp = CastField<FByteProperty>(Property);/* BP定義だとこっちみたい… */

	const FStructProperty* StructProp = CastField<FStructProperty>(Property);

	if(EnumProp != nullptr)
	{
		PropertyInfo->EnumType = EnumProp->GetEnum();
	}
	else if(ByteProp != nullptr)
	{
		PropertyInfo->EnumType = ByteProp->Enum;
	}
	else if( StructProp != nullptr )
	{
		PropertyInfo->Struct = StructProp->Struct;
	}

	ObjectProperties.Add(PropertyInfo);

	ObjectProperties.Sort([](const TSharedPtr<FGDMObjectPropertyInfo>& A,const TSharedPtr<FGDMObjectPropertyInfo>& B)
	{
		return A->DisplayPriority >= B->DisplayPriority;
	});
	
	if (!PropertySaveKey.IsEmpty())
	{
		/* プロパティ型に応じた値を一時的に保存 */
		FVariant OldValueVariant;

		void* ValuePtr = Property->ContainerPtrToValuePtr<void*>(TargetObject);
		switch (PropertyType)
		{
			case EGDMPropertyType::GDM_Bool:	{ SET_VARIANT_PROPERTY(FBoolProperty) break;	}
			case EGDMPropertyType::GDM_Int:		{ SET_VARIANT_PROPERTY(FIntProperty) break;		}
			case EGDMPropertyType::GDM_Float:	{ SET_VARIANT_PROPERTY(FFloatProperty) break;	}
			case EGDMPropertyType::GDM_Enum:	{ SET_VARIANT_ENUM_PROPERTY() break;			}
			case EGDMPropertyType::GDM_Byte:	{ SET_VARIANT_PROPERTY(FByteProperty) break;	}
			case EGDMPropertyType::GDM_String:	{ SET_VARIANT_PROPERTY(FStrProperty) break;		}
			case EGDMPropertyType::GDM_Vector:  { SET_VARIANT_STRUCT_PROPERTY(FVector) break;	}
			case EGDMPropertyType::GDM_Vector2D:{ SET_VARIANT_STRUCT_PROPERTY(FVector2D) break;	}
			case EGDMPropertyType::GDM_Rotator:	{ SET_VARIANT_STRUCT_PROPERTY(FRotator) break;	}
			default:
				UE_LOG(LogGDM, Warning, TEXT("Unsupported property type for old value caching"));
		}
		
		/* 保存キーを指定してるため、既に一致する情報があればそれをプロパティにセットを試みる */
		if (!GetPropertyJsonSystemComponent()->ApplyJsonToObject(PropertySaveKey, TargetObject, PropertyName.ToString()))
		{
			/* 失敗、データがないので現状の値をJsonに書き込み */
			GetPropertyJsonSystemComponent()->AddPropertyToJson(PropertySaveKey, TargetObject, PropertyName.ToString());
		}
		else
		{
			switch (PropertyType)
			{
				case EGDMPropertyType::GDM_Bool:	{ CALL_PROPERTY_DISPATCHER(Bool, FBoolProperty, bool) break;		}
				case EGDMPropertyType::GDM_Int:		{ CALL_PROPERTY_DISPATCHER(Int, FIntProperty, int32) break;			}
				case EGDMPropertyType::GDM_Float:	{ CALL_PROPERTY_DISPATCHER(Float, FFloatProperty, float) break;		}
				case EGDMPropertyType::GDM_Enum:	{ CALL_ENUM_PROPERTY_DISPATCHER() break;							}
				case EGDMPropertyType::GDM_Byte:    { CALL_PROPERTY_DISPATCHER(Byte, FByteProperty, uint8) break;		}
				case EGDMPropertyType::GDM_String:  { CALL_PROPERTY_DISPATCHER(String, FStrProperty, FString) break;	}
				case EGDMPropertyType::GDM_Vector:  { CALL_STRUCT_PROPERTY_DISPATCHER(Vector, FVector); break;			}
				case EGDMPropertyType::GDM_Vector2D:{ CALL_STRUCT_PROPERTY_DISPATCHER(Vector2D, FVector2D); break;		}
				case EGDMPropertyType::GDM_Rotator:	{ CALL_STRUCT_PROPERTY_DISPATCHER(Rotator, FRotator); break;		}
				default:
					UE_LOG(LogGDM, Warning, TEXT("Unsupported property type for old value caching"));
			}
		}
	}
	
	return true;
}

bool AGameDebugMenuManager::RegisterObjectFunction(UObject* TargetObject, const FName FunctionName, const FGDMGameplayCategoryKey& CategoryKey,const FText& DisplayFunctionName,const FText& Description,const int32& DisplayPriority)
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

	const TSharedPtr<FGDMObjectFunctionInfo> FunctionInfo  = MakeShareable(new FGDMObjectFunctionInfo);
	FunctionInfo->CategoryKey                              = CategoryKey;
	FunctionInfo->Name                                     = (DisplayFunctionName.IsEmpty() != false) ? FText::FromName(FunctionName) : DisplayFunctionName;
	FunctionInfo->TargetObject                             = TargetObject;
	FunctionInfo->FunctionName                             = FunctionName;
	FunctionInfo->TargetFunction                           = Function;
	FunctionInfo->Description						       = Description;
	FunctionInfo->DisplayPriority					       = DisplayPriority;

	ObjectFunctions.Add(FunctionInfo);

	ObjectFunctions.Sort([](const TSharedPtr<FGDMObjectFunctionInfo>& A,const TSharedPtr<FGDMObjectFunctionInfo>& B)
	{
		return A->DisplayPriority >= B->DisplayPriority;
	});

	return true;
}

UObject* AGameDebugMenuManager::GetObjectProperty(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutPropertySaveKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& OutPropertyUIConfigInfo)
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
	OutPropertySaveKey		= ObjProp->PropertySaveKey;
	OutDisplayPropertyName	= ObjProp->Name;
	OutDescription			= ObjProp->Description;
	OutPropertyName			= ObjProp->PropertyName;
	OutPropertyUIConfigInfo = ObjProp->ConfigInfo;
	OutPropertyType			= GetPropertyType(Property);

	if(OutPropertyType == EGDMPropertyType::GDM_Enum)
	{
		OutEnumPathName = ObjProp->EnumType->GetPathName();
	}
	else if(OutPropertyType == EGDMPropertyType::GDM_Byte)
	{
		if(ObjProp->EnumType.IsValid())
		{
			/* Enumならセット */
			OutPropertyType = EGDMPropertyType::GDM_Enum;
			OutEnumPathName = ObjProp->EnumType->GetPathName();
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

int32 AGameDebugMenuManager::GetNumObjectProperties() const
{
	return ObjectProperties.Num();
}

int32 AGameDebugMenuManager::GetNumObjectFunctions() const
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

	const FName CurrentDebugMenuLanguage = GetCurrentDebugMenuLanguage();
	
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

	const FName Old = CurrentDebugMenuLanguage;

	/* 言語を切り替えをし保存。その後テーブルを読み直す */
	{
		GetPropertyJsonSystemComponent()->SetSingleStringToJson(TEXT("DebugMenuDirectStringKey"), bCurrentDebugMenuDirectStringKey ? TEXT("True") : TEXT("False"));
		GetPropertyJsonSystemComponent()->SetSingleStringToJson(TEXT("DebugMenuLanguage"), LanguageKey.ToString());
		GetSaveSystemComponent()->SaveDebugMenuFile();
		SyncLoadDebugMenuStringTables(LanguageKey);
	}

	TArray<UWidget*> ChildWidgets;
	for(const auto ViewportWidget : ViewportDebugMenuWidgets )
	{
		/* Viewportに追加 Widget 内にあるすべてのTextBlockとDebugMenuWidgetを更新 */
		{
			ViewportWidget->GetWidgetChildrenOfClass(UGDMTextBlock::StaticClass(), ChildWidgets, false);

			for(const auto ChildWidget : ChildWidgets )
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

			for(const auto ChildWidget : ChildWidgets )
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

TArray<UGameDebugMenuWidget*> AGameDebugMenuManager::GetViewportDebugMenuWidgets()
{
	return ViewportDebugMenuWidgets;
}

FName AGameDebugMenuManager::GetCurrentDebugMenuLanguage() const
{
	return *GetPropertyJsonSystemComponent()->GetSingleStringFromJson(TEXT("DebugMenuLanguage"), UGameDebugMenuSettings::Get()->DefaultGameDebugMenuLanguage.ToString());
}

void AGameDebugMenuManager::CallExecuteConsoleCommandDispatcher(const FString& Command)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnExecuteConsoleCommandDispatcher.Broadcast(Command);
	}
}

void AGameDebugMenuManager::CallShowDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnShowDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallHideDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnHideDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallRegisterInputSystemEventDispatcher(UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnRegisterInputSystemDispatcher.Broadcast(TargetObject);
	}
}

void AGameDebugMenuManager::CallUnregisterInputSystemEventDispatcher(UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnUnregisterInputSystemDispatcher.Broadcast(TargetObject);
	}
}

void AGameDebugMenuManager::OnWidgetAdded(UWidget* AddWidget, ULocalPlayer* Player)
{
	if(UGameDebugMenuWidget* W = Cast<UGameDebugMenuWidget>(AddWidget))
	{
		ViewportDebugMenuWidgets.AddUnique(W);
	}
}

void AGameDebugMenuManager::OnWidgetRemoved(UWidget* RemoveWidget)
{
	if(UGameDebugMenuWidget* W = Cast<UGameDebugMenuWidget>(RemoveWidget))
	{
		ViewportDebugMenuWidgets.RemoveSingle(W);
	}
}

void AGameDebugMenuManager::CallExecuteProcessEventDispatcher(const FName& FunctionName, UObject* TargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnExecuteProcessEventDispatcher.Broadcast(FunctionName, TargetObject);
	}
}

void AGameDebugMenuManager::CallChangePropertyBoolDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, bool New, bool Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnChangePropertyBoolDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyIntDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, int32 New, int32 Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnChangePropertyIntDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyFloatDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, float New, float Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnChangePropertyFloatDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyByteDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, uint8 New, uint8 Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnChangePropertyByteDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyStringDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FString New, FString Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents)
	{
		Component->OnChangePropertyStringDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyVectorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector New, FVector Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnChangePropertyVectorDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyVector2DDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FVector2D New, FVector2D Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnChangePropertyVector2DDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangePropertyRotatorDispatcher(const FName& PropertyName, UObject* PropertyOwnerObject, FRotator New, FRotator Old, const FString& PropertySaveKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnChangePropertyRotatorDispatcher.Broadcast(PropertyName, PropertyOwnerObject, New, Old, PropertySaveKey);
	}
}

void AGameDebugMenuManager::CallChangeActiveInputObjectDispatcher(UObject* NewTargetObject, UObject* OldTargetObject)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnChangeActiveInputObjectDispatcher.Broadcast(NewTargetObject, OldTargetObject);
	}
}

void AGameDebugMenuManager::CallChangeDebugMenuLanguageDispatcher(const FName& NewLanguageKey, const FName& OldLanguageKey)
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnChangeDebugMenuLanguageDispatcher.Broadcast(NewLanguageKey, OldLanguageKey);
	}
}

void AGameDebugMenuManager::CallStartScreenshotRequestDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnStartScreenshotRequestDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallScreenshotRequestProcessedDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnScreenshotRequestProcessedDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallLoadedDebugMenuDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnLoadedDebugMenuDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallSavedDebugMenuDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnSavedDebugMenuDispatcher.Broadcast();
	}
}

void AGameDebugMenuManager::CallDeletedDebugMenuDispatcher()
{
	TArray<UGDMListenerComponent*> ListenerComponents;
	UGDMListenerComponent::GetAllListenerComponents(GetWorld(), ListenerComponents);

	for(const auto& Component : ListenerComponents )
	{
		Component->OnDeletedDebugMenuDispatcher.Broadcast();
	}
}
