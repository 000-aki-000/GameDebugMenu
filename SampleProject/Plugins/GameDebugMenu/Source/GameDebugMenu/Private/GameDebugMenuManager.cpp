/**
* Copyright (c) 2020 akihiko moroi
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
#include "Blueprint/GameViewportSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Application/SlateApplication.h"

#include "GameDebugMenuSettings.h"
#include "GameDebugMenuFunctions.h"
#include "Log/GDMOutputDevice.h"
#include "Component/GDMListenerComponent.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Component/GDMScreenshotRequesterComponent.h"
#include "Component/GDMLocalizeStringComponent.h"
#include "Component/GDMPropertyJsonSystemComponent.h"
#include "Component/GDMSaveSystemComponent.h"
#include "ConsoleCommand/GDMConsoleCommandValueProviderComponent.h"
#include "Input/GDMInputSystemComponent.h"
#include "Widgets/GameDebugMenuRootWidget.h"
#include "Widgets/GDMTextBlock.h"
#include "Data/GameDebugMenuManagerAsset.h"
#include "Favorite/GDMFavoriteSystemComponent.h"


/********************************************************************/
/* AGameDebugMenuManager										*/
/********************************************************************/

AGameDebugMenuManager::AGameDebugMenuManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuInputSystemComponent(nullptr)
	, ScreenshotRequesterComponent(nullptr)
	, PropertyJsonSystemComponent(nullptr)
	, SaveSystemComponent(nullptr)
	, LocalizeStringComponent(nullptr)
	, ListenerComponent(nullptr)
	, InitializeManagerHandle()
	, bInitializedManager(false)
	, MenuAsset(nullptr)
	, ConsoleCommandSetAsset(nullptr)
	, bShowDebugMenu(false)
	, bCachedGamePaused(false)
	, bCachedShowMouseCursor(false)
	, bWaitToCaptureBeforeOpeningDebugReportMenu(false)
	, CachedNavigationConfigs()
	, ObjectProperties()
	, ObjectFunctions()
	, DebugMenuRootWidget(nullptr)
	, DebugMenuInstances()
	, OutputLog(nullptr)
{
	DebugMenuInputSystemComponent = CreateDefaultSubobject<UGDMInputSystemComponent>(TEXT("DebugMenuInputSystemComponent"));
	ScreenshotRequesterComponent  = CreateDefaultSubobject<UGDMScreenshotRequesterComponent>(TEXT("ScreenshotRequesterComponent"));
	PropertyJsonSystemComponent   = CreateDefaultSubobject<UGDMPropertyJsonSystemComponent>(TEXT("PropertyJsonSystemComponent"));
	SaveSystemComponent			  = CreateDefaultSubobject<UGDMSaveSystemComponent>(TEXT("SaveSystemComponent"));
	FavoriteSystemComponent		  = CreateDefaultSubobject<UGDMFavoriteSystemComponent>(TEXT("FavoriteSystemComponent"));
	ConsoleCommandValueProviderComponent = CreateDefaultSubobject<UGDMConsoleCommandValueProviderComponent>(TEXT("ConsoleCommandValueProviderComponent"));;
	LocalizeStringComponent		  = CreateDefaultSubobject<UGDMLocalizeStringComponent>(TEXT("LocalizeStringComponent"));
	ListenerComponent             = CreateDefaultSubobject<UGDMListenerComponent>(TEXT("ListenerComponent"));

	PrimaryActorTick.bCanEverTick			= true;
	PrimaryActorTick.bStartWithTickEnabled	= true;
	PrimaryActorTick.bTickEvenWhenPaused	= true;
	SetCanBeDamaged(false);
	SetHidden(true);
	InputPriority                           = TNumericLimits<int32>::Max();
	bBlockInput                             = false;
	bReplicates                             = true;
	bAlwaysRelevant                         = true;
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(1);/* デフォルトのPlayerStateと同じかんじにしとく */
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
}

void AGameDebugMenuManager::BeginPlay()
{
	OutputLog = MakeShared<FGDMOutputDevice>();
	
	/* 他で参照される前にロードは処理しとく */
	GetSaveSystemComponent()->LoadDebugMenuFile();
	
	Super::BeginPlay();

	UGameDebugMenuFunctions::RegisterGameDebugMenuManagerInstance(this);
	
	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Call BeginPlay"), 4.0f);

	/* managerのBeginplayがちゃんと完了後に処理↓ */
	GetWorld()->GetTimerManager().SetTimer(InitializeManagerHandle, FTimerDelegate::CreateLambda([this]()
	{
		OnInitializeManager();
	}), 0.01f, true);
}

void AGameDebugMenuManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Call EndPlay"), 4.0f);
	
	OutputLog.Reset();

	if(EndPlayReason != EEndPlayReason::EndPlayInEditor && EndPlayReason != EEndPlayReason::Quit)
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InitializeManagerHandle);
		}
		
		EnabledNavigationConfigs();
		
		if(IsValid(DebugMenuRootWidget))
		{
			DebugMenuRootWidget->RemoveFromParent();
			DebugMenuRootWidget = nullptr;
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

UGDMFavoriteSystemComponent* AGameDebugMenuManager::GetFavoriteSystemComponent() const
{
	return FavoriteSystemComponent;
}

UGDMConsoleCommandValueProviderComponent* AGameDebugMenuManager::GetConsoleCommandValueProviderComponent() const
{
	return ConsoleCommandValueProviderComponent;
}

UGDMLocalizeStringComponent* AGameDebugMenuManager::GetLocalizeStringComponent() const
{
	return LocalizeStringComponent;
}

UGDMListenerComponent* AGameDebugMenuManager::GetListenerComponent() const
{
	return ListenerComponent;
}

bool AGameDebugMenuManager::IsInitializedManager() const
{
	return bInitializedManager;
}

void AGameDebugMenuManager::OnInitializeManager()
{
	if (bInitializedManager)
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: The manager has already been initialized"), 0.6f);
		return;
	}
	
	if (GetOwner() == nullptr)
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: Not found Owner"), 0.6f);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!IsValid(PC))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: The owner is not a PlayerController"), 0.6f);
		return;
	}

	if (!IsValid(PC->GetLocalPlayer()))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("AGameDebugMenuManager: No LocalPlayer"), 0.6f);
		return;
	}

	bInitializedManager = true;

	GetLocalizeStringComponent()->SetJsonSystemComponentValue(GetPropertyJsonSystemComponent());
	
	GetLocalizeStringComponent()->SyncLoadDebugMenuStringTables();
	
	if( !UKismetSystemLibrary::IsDedicatedServer(this) )
	{
		GetFavoriteSystemComponent()->Initialize(MenuAsset);
		
		CreateDebugMenuRootWidget();
		
		GetDebugMenuInputSystemComponent()->Initialize(MenuAsset);
		
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UGameDebugMenuWidget::StaticClass(), true);
						
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
	
	if (!IsValid(PC->CheatManager))
	{
		if (GetNetMode() == NM_Client)
		{
			/* クライアントでも利用できるように常に有効化 */
			PC->EnableCheats();
		}
	}
	
	TArray<FString> CommandHistory = GetPropertyJsonSystemComponent()->GetCustomStringArray(TEXT("CommandHistory"));
	for(const auto& Command : CommandHistory )
	{
		PC->ConsoleCommand(Command);
	}

	GetWorld()->GetTimerManager().ClearTimer(InitializeManagerHandle);

	EnableInput(PC);

	OnInitializeManagerBP();
}

void AGameDebugMenuManager::CreateDebugMenuRootWidget()
{
	if( !IsValid(MenuAsset) )
	{
		UE_LOG(LogGDM, Warning, TEXT("CreateDebugMenuRootWidget: Not found MenuAsset"));
		return;
	}
	
	if( IsValid(DebugMenuRootWidget) )
	{
		DebugMenuRootWidget->RemoveFromParent();
	}

	DebugMenuRootWidget = Cast<UGameDebugMenuRootWidget>(UWidgetBlueprintLibrary::Create(this, MenuAsset->DebugMenuRootWidgetClass, GetOwnerPlayerController()));
	DebugMenuRootWidget->SetDebugMenuManager(this);
	DebugMenuRootWidget->AddToViewport(MenuAsset->RootWidgetZOrder);
	DebugMenuRootWidget->SetVisibility(ESlateVisibility::Collapsed);
	DebugMenuRootWidget->InitializeInputComponent();
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
	check(IsValid(MenuAsset));
	if(MenuAsset->bGamePause)
	{
		bCachedGamePaused = UGameplayStatics::IsGamePaused(this);
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AGameDebugMenuManager::RestoreGamePause() const
{
	check(IsValid(MenuAsset));
	if(MenuAsset->bGamePause)
	{
		UGameplayStatics::SetGamePaused(this, bCachedGamePaused);
	}
}

void AGameDebugMenuManager::OnScreenshotRequestProcessed()
{
	bWaitToCaptureBeforeOpeningDebugReportMenu = false;

	GetListenerComponent()->OnScreenshotRequestProcessedDispatcher.RemoveDynamic(this, &AGameDebugMenuManager::OnScreenshotRequestProcessed);

	SetIgnoreInput(false);

	GetDebugMenuInputSystemComponent()->OnOpenMenu();
	
	TArray<UGameDebugMenuWidget*> DebugMenuWidgets = GetViewportDebugMenuWidgets();
	for(const auto ViewportWidget : DebugMenuWidgets )
	{
		if( ViewportWidget->IsActivateDebugMenu() )
		{
			ViewportWidget->OnShowingMenu(/* bRequestDebugMenuManager */true);
		}
	}
	
	GetDebugMenuRootWidget()->ShowDebugReport();

	CallShowDispatcher();
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
	
	if(!IsValid(DebugMenuRootWidget))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("ShowDebugMenu: Not found DebugMenuRootWidget"), 4.0f);
		return false;
	}

	bShowDebugMenu = true;

	APlayerController* PC = GetOwnerPlayerController();
	DisabledNavigationConfigs();
	EnableShowMouseCursorFlag(PC);
	TryEnableGamePause();

	bool bShow = true;

	if( !GetDefault<UGameDebugMenuSettings>()->bDisableScreenCaptureProcessingWhenOpeningDebugMenu )
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
		GetDebugMenuInputSystemComponent()->OnOpenMenu();
		
		TArray<UGameDebugMenuWidget*> DebugMenuWidgets = GetViewportDebugMenuWidgets();
		for(const auto ViewportWidget : DebugMenuWidgets )
		{
			if( ViewportWidget->IsActivateDebugMenu() )
			{
				ViewportWidget->OnShowingMenu(/* bRequestDebugMenuManager */true);
			}
		}

		CallShowDispatcher();

		UE_LOG(LogGDM, Log, TEXT("ShowDebugMenu"));
	}

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
	
	if(!IsValid(DebugMenuRootWidget))
	{
		UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("HideDebugMenu: Not found DebugMenuWidget"), 4.0f);
		return;
	}
	
	GetDebugMenuInputSystemComponent()->OnCloseMenu();
	
	bShowDebugMenu = false;

	APlayerController* PC = GetOwnerPlayerController();
	EnabledNavigationConfigs();
	RestoreShowMouseCursorFlag(PC);
	RestoreGamePause();
	GetSaveSystemComponent()->SaveDebugMenuFile();
	
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

APlayerController* AGameDebugMenuManager::GetOwnerPlayerController() const
{
	checkf(IsValid(GetOwner()), TEXT("No owner"));
	checkf(IsValid(Cast<APlayerController>(GetOwner())), TEXT("Owner needs to be PlayerController"));
	return Cast<APlayerController>(GetOwner());
}

UGameDebugMenuRootWidget* AGameDebugMenuManager::GetDebugMenuRootWidget()
{
	return DebugMenuRootWidget;
}

int32 AGameDebugMenuManager::GetAllDebugMenuKeys(TArray<FString>& OutKeys)
{
	if( IsValid(MenuAsset) )
	{
		OutKeys = MenuAsset->DebugMenuRegistrationOrder;
	}
	else
	{
		UE_LOG(LogGDM, Warning, TEXT("GetAllDebugMenuKeys: Not found MenuAsset"));
	}

	return OutKeys.Num();
}

TSubclassOf<UGameDebugMenuWidget> AGameDebugMenuManager::GetDebugMenuWidgetClass(const FString& Key)
{
	if( IsValid(MenuAsset) )
	{
		if(const TSubclassOf<UGameDebugMenuWidget>* WidgetClass = MenuAsset->DebugMenuClasses.Find(Key) )
		{
			return (*WidgetClass);
		}
	}
	else
	{
		UE_LOG(LogGDM, Warning, TEXT("GetAllDebugMenuKeys: Not found MenuAsset"));
	}

	return nullptr;
}

FString AGameDebugMenuManager::GetDebugMenuWidgetKey(const UGameDebugMenuWidget* Widget)
{
	for (const auto& Pair : DebugMenuInstances)
	{
		if( Pair.Value == Widget )
		{
			return Pair.Key;
		}
	}

	return TEXT("");
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

void AGameDebugMenuManager::ClearCommandHistory()
{
	OutputLog->ClearCommandHistory();
}

EGDMPropertyType AGameDebugMenuManager::GetPropertyType(const FProperty* TargetProperty) const
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
		if (!GetPropertyJsonSystemComponent()->ApplyJsonToObjectProperty(PropertySaveKey, TargetObject, PropertyName.ToString()))
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

bool AGameDebugMenuManager::RegisterObjectFunction(UObject* TargetObject, const FName FunctionName, const FGDMGameplayCategoryKey& CategoryKey, const FString& FunctionSaveKey, const FText& DisplayFunctionName,const FText& Description,const int32& DisplayPriority)
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
	FunctionInfo->FunctionSaveKey					       = FunctionSaveKey;
	
	ObjectFunctions.Add(FunctionInfo);

	ObjectFunctions.Sort([](const TSharedPtr<FGDMObjectFunctionInfo>& A,const TSharedPtr<FGDMObjectFunctionInfo>& B)
	{
		return A->DisplayPriority >= B->DisplayPriority;
	});

	if (!FunctionSaveKey.IsEmpty())
	{
		/* 保存キーを指定してるため、既に一致する情報があればそれをプロパティにセットを試みる */
		if (!GetPropertyJsonSystemComponent()->HaveFunctionInJson(FunctionSaveKey, TargetObject, FunctionName.ToString()))
		{
			/* 失敗、データがないので現状の値をJsonに書き込み */
			GetPropertyJsonSystemComponent()->AddFunctionToJson(FunctionSaveKey, TargetObject, FunctionName.ToString());
		}
	}
	
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

UObject* AGameDebugMenuManager::GetObjectFunction(const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutFunctionSaveKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName)
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
	OutFunctionSaveKey	   = ObjFunc->FunctionSaveKey;
	
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

UObject* AGameDebugMenuManager::TryGetObjectProperty(const FString& InPropertySaveKey, const FString& InPropertyName, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayPropertyName, FText& OutDescription, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& OutPropertyUIConfigInfo) const
{
	OutPropertyType = EGDMPropertyType::GDM_Null;
	
	for (const auto& ObjProp : ObjectProperties)
	{
		if(!ObjProp->TargetObject.IsValid())
		{
			continue;
		}

		const auto Property = ObjProp->TargetProperty;
		if(Property == nullptr)
		{
			continue;
		}

		if (ObjProp->PropertySaveKey != InPropertySaveKey || ObjProp->PropertyName != InPropertyName)
		{
			continue;
		}
		
		OutCategoryKey			= ObjProp->CategoryKey;
		OutDisplayPropertyName	= ObjProp->Name;
		OutDescription			= ObjProp->Description;
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

	return nullptr;
}

UObject* AGameDebugMenuManager::TryGetObjectFunction(const FString& InFunctionSaveKey, const FString& InFunctionName, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayFunctionName, FText& OutDescription) const
{
	for (const auto& ObjFunc : ObjectFunctions)
	{
		if(!ObjFunc->TargetObject.IsValid())
		{
			continue;
		}

		const auto Function = ObjFunc->TargetFunction;
		if(!Function.IsValid())
		{
			continue;
		}

		if (ObjFunc->FunctionSaveKey != InFunctionSaveKey || ObjFunc->FunctionName != InFunctionName)
		{
			continue;
		}
		
		OutCategoryKey			= ObjFunc->CategoryKey;
		OutDisplayFunctionName	= ObjFunc->Name;
		OutDescription			= ObjFunc->Description;
		
		return ObjFunc->TargetObject.Get();
	}

	return nullptr;
}

void AGameDebugMenuManager::AddDebugMenuPCProxyComponent(APlayerController* PlayerController)
{
	check(IsValid(MenuAsset));
	
	if (!IsValid(MenuAsset->DebugMenuPCProxyComponentClass))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddDebugMenuPCProxyComponent: Not found DebugMenuPCProxyComponentClass"));
		return;
	}

	UGDMPlayerControllerProxyComponent* NewComponent = Cast<UGDMPlayerControllerProxyComponent>( PlayerController->AddComponentByClass(MenuAsset->DebugMenuPCProxyComponentClass, false, FTransform::Identity, true));
	check(IsValid(NewComponent));
	NewComponent->DebugMenuManager = this;
	PlayerController->FinishAddComponent(NewComponent, false, FTransform::Identity);
}

void AGameDebugMenuManager::SetIgnoreInput(bool bNewInput)
{
	GetDebugMenuInputSystemComponent()->SetIgnoreInput(bNewInput);
}

void AGameDebugMenuManager::ResetIgnoreInput()
{
	GetDebugMenuInputSystemComponent()->ResetIgnoreInput();
}

bool AGameDebugMenuManager::IsInputIgnored() const
{
	return GetDebugMenuInputSystemComponent()->IsInputIgnored();
}

void AGameDebugMenuManager::ChangeDebugMenuLanguage(FName LanguageKey, bool bForcedUpdate)
{
	UE_LOG(LogGDM, Verbose, TEXT("Call ChangeDebugMenuLanguage LanguageKey:%s bForcedUpdate:%d"), *LanguageKey.ToString(), bForcedUpdate);

	const FName CurrentDebugMenuLanguage = GetLocalizeStringComponent()->GetCurrentDebugMenuLanguage();
	
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

	/* 言語を切り替えをして保存。その後テーブルを読み直す */
	{
		GetLocalizeStringComponent()->SetToJsonSystemComponent(GetPropertyJsonSystemComponent(), LanguageKey.ToString());
		GetSaveSystemComponent()->SaveDebugMenuFile();
		GetLocalizeStringComponent()->SyncLoadDebugMenuStringTables();
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

TArray<UGameDebugMenuWidget*> AGameDebugMenuManager::GetViewportDebugMenuWidgets()
{
	return ViewportDebugMenuWidgets;
}

void AGameDebugMenuManager::ChangeConsoleCommandSetAsset(UGDMConsoleCommandSetAsset* NewCommandSetAsset)
{
	ConsoleCommandSetAsset = NewCommandSetAsset;
}

UGDMConsoleCommandSetAsset* AGameDebugMenuManager::GetConsoleCommandSetAsset() const
{
	return ConsoleCommandSetAsset;
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
