/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuFunctions.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "CoreGlobals.h"
#include "Misc/ConfigCacheIni.h"
#include <GeneralProjectSettings.h>

#include "GameDebugMenuSettings.h"
#include "Component/GDMLocalizeStringComponent.h"
#include "Data/GameDebugMenuMasterAsset.h"
#include "Input/GDMInputSystemComponent.h"

TArray< TWeakObjectPtr<AGameDebugMenuManager> > GGameDebugMenuManagers;
TWeakObjectPtr<AGameDebugMenuManager> UGameDebugMenuFunctions::CurrentGameDebugMenuManager = nullptr;
TArray<FGDMPendingObjectData> UGameDebugMenuFunctions::RegisterPendingProperties;
TArray<FGDMPendingObjectData> UGameDebugMenuFunctions::RegisterPendingFunctions;
FDelegateHandle UGameDebugMenuFunctions::ActorSpawnedDelegateHandle;
bool UGameDebugMenuFunctions::bDisableGameDebugMenu = false;

UGameDebugMenuFunctions::UGameDebugMenuFunctions(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if(!IsRunningCommandlet())
	{
		auto& ConsoleManager = IConsoleManager::Get();
		const FString ShowCommand = TEXT("GDM.Show");
		const FString HideCommand = TEXT("GDM.Hide");
		const FString ToggleMenuCommand = TEXT("GDM.ToggleMenu");
		const FString ToggleInputCommand = TEXT("GDM.ToggleInput");

		if(ConsoleManager.FindConsoleObject(*ShowCommand) == nullptr)
		{
			ConsoleManager.RegisterConsoleCommand(*ShowCommand, TEXT("Show Game Debug Menu"), FConsoleCommandDelegate::CreateStatic(UGameDebugMenuFunctions::ShowDebugConsoleCommand), ECVF_Default);
		}
		if(ConsoleManager.FindConsoleObject(*HideCommand) == nullptr)
		{
			ConsoleManager.RegisterConsoleCommand(*HideCommand, TEXT("Hide Game Debug Menu"), FConsoleCommandDelegate::CreateStatic(UGameDebugMenuFunctions::HideDebugConsoleCommand), ECVF_Default);
		}
		if(ConsoleManager.FindConsoleObject(*ToggleMenuCommand) == nullptr)
		{
			ConsoleManager.RegisterConsoleCommand(*ToggleMenuCommand, TEXT("Toggle Game Debug Menu"), FConsoleCommandDelegate::CreateStatic(UGameDebugMenuFunctions::ToggleDebugConsoleCommand), ECVF_Default);
		}
		if( ConsoleManager.FindConsoleObject(*ToggleInputCommand) == nullptr )
		{
			ConsoleManager.RegisterConsoleCommand(*ToggleInputCommand, TEXT("Toggle Game Debug Menu InputSystem Log"), FConsoleCommandDelegate::CreateStatic(UGameDebugMenuFunctions::ToggleInputSystemLog), ECVF_Default);
		}
	}
}

void UGameDebugMenuFunctions::RegisterGameDebugMenuManagerInstance(AGameDebugMenuManager* RegisterManager)
{
	GGameDebugMenuManagers.AddUnique(RegisterManager);

	if( !UKismetSystemLibrary::IsServer(RegisterManager) )
	{
		/* Client */
		OnActorSpawnedClientWaitManager(RegisterManager);
	}
}

void UGameDebugMenuFunctions::UnregisterGameDebugMenuManagerInstance(AGameDebugMenuManager* UnregisterManager)
{
	GGameDebugMenuManagers.Remove(UnregisterManager);

	if( UnregisterManager == CurrentGameDebugMenuManager )
	{
		CurrentGameDebugMenuManager = nullptr;
	}
}

bool UGameDebugMenuFunctions::TryCreateDebugMenuManager(APlayerController* PlayerController, FString DebugMenuManagerClassName)
{
	bool bServer    = UKismetSystemLibrary::IsServer(PlayerController);
	FString NetMode = (bServer ? TEXT("Server") : TEXT("Client"));

	UWorld* World = GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull);
	if(World == nullptr)
	{
		UE_LOG(LogGDM, Warning, TEXT("TryCreateDebugMenuManager: Not found world[%s]"), *NetMode);
		return false;
	}

	if (DebugMenuManagerClassName.IsEmpty())
	{
		UE_LOG(LogGDM, Warning, TEXT("TryCreateDebugMenuManager: class name empty[%s]"), *NetMode);
		return false;
	}

	const UGameDebugMenuMasterAsset* MasterAsset = GetDefault<UGameDebugMenuSettings>()->GetMasterAsset();
	if (!IsValid(MasterAsset))
	{
		UE_LOG(LogGDM, Warning, TEXT("TryCreateDebugMenuManager: Not found MasterAsset[%s]"), *NetMode);

		/* 生成ができない環境なので無効フラグを立てる */
		bDisableGameDebugMenu = true;
		CurrentGameDebugMenuManager = nullptr;
		RegisterPendingProperties.Empty();
		RegisterPendingFunctions.Empty();
		return false;
	}
	
	const auto ManagerClass = MasterAsset->GetGameDebugMenuManagerSoftClass(DebugMenuManagerClassName);
	UClass* DebugMenuManagerClass = ManagerClass.LoadSynchronous();
	if(DebugMenuManagerClass == nullptr)
	{
		FString ErrorStr = TEXT("DebugMenuManagerClass NotFound LoadError");
		ErrorStr += TEXT("[");
		ErrorStr += NetMode;
		ErrorStr += TEXT("]");

		UE_LOG(LogGDM, Warning, TEXT("TryCreateDebugMenuManager: Failed Class name [%s] %s"), *NetMode, *DebugMenuManagerClassName);
		UKismetSystemLibrary::PrintString(PlayerController, ErrorStr, true, true, FLinearColor::Red, 10.0f);

		/* 生成ができない環境なので無効フラグを立てる */
		bDisableGameDebugMenu = true;
		CurrentGameDebugMenuManager = nullptr;
		RegisterPendingProperties.Empty();
		RegisterPendingFunctions.Empty();
		return false;
	}

	bDisableGameDebugMenu = false;
	
	if(!bServer)
	{
		/* Client */
		if( !IsValid(GetGameDebugMenuManager(PlayerController, false)) )
		{
			/* まだ生成してもらってないので待つ */
			UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Wait spawn[%s]"), *NetMode);
			return true;
		}

		CurrentGameDebugMenuManager = GetGameDebugMenuManager(PlayerController, false);
	}
	else
	{
		/* Server */
		if (IsValid(GetGameDebugMenuManager(PlayerController, false)))
		{
			UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Created manager[%s]"), *NetMode);
			return true;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = PlayerController;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		CurrentGameDebugMenuManager = World->SpawnActor<AGameDebugMenuManager>(DebugMenuManagerClass, FTransform::Identity, SpawnParams);

		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (!IsValid(PC))
			{
				continue;
			}
			/* PlayerControllerにコンポーネントを追加 */
			CurrentGameDebugMenuManager->AddDebugMenuPCProxyComponent(PC);
		}

		ActorSpawnedDelegateHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateStatic(&UGameDebugMenuFunctions::OnActorSpawnedServer));
	}

	/* マネージャー生成前に登録処理したプロパティ群を追加する */
	for(int32 Index = 0; Index < RegisterPendingProperties.Num(); ++Index)
	{
		if(RegisterPendingProperties.IsValidIndex(Index))
		{
			const auto& PendingData = RegisterPendingProperties[Index];
			if(PendingData.TargetObject.IsValid())
			{
				RegisterGDMObjectProperty(PendingData.TargetObject.Get()
					,PendingData.ConfigInfo
					,PendingData.TargetName
					,PendingData.CategoryKey
					,PendingData.SaveKey
					,PendingData.DisplayPropertyName
					,PendingData.Description
					,PendingData.DisplayPriority
				);
			}
		}
	}

	/* マネージャー生成前に登録処理した関数群を追加する */
	for(int32 Index = 0; Index < RegisterPendingFunctions.Num(); ++Index)
	{
		if(RegisterPendingFunctions.IsValidIndex(Index))
		{
			const auto& PendingData = RegisterPendingFunctions[Index];
			if(PendingData.TargetObject.IsValid())
			{
				RegisterGDMObjectFunction(PendingData.TargetObject.Get(),
										  PendingData.TargetName,
										  PendingData.CategoryKey,
										  PendingData.SaveKey,
										  PendingData.DisplayPropertyName,
										  PendingData.Description,
										  PendingData.DisplayPriority
				);
			}
		}
	}

	RegisterPendingProperties.RemoveAll([&](const FGDMPendingObjectData& PropertyData)
	{
		if(!PropertyData.TargetObject.IsValid())
		{
			return true;
		}
		return (PropertyData.TargetObject->GetWorld() == World );
	});

	RegisterPendingFunctions.RemoveAll([&](const FGDMPendingObjectData& FunctionData)
	{
		if(!FunctionData.TargetObject.IsValid())
		{
			return true;
		}
		return (FunctionData.TargetObject->GetWorld() == World );
	});

	UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Success %s"), *NetMode);

	return true;
}

bool UGameDebugMenuFunctions::DestroyDebugMenuManager(APlayerController* PlayerController)
{
	UWorld* World = GEngine->GetWorldFromContextObject(PlayerController, EGetWorldErrorMode::LogAndReturnNull);
	if(World != nullptr)
	{
		World->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
		RegisterPendingProperties.RemoveAll([&](const FGDMPendingObjectData& PropertyData)
			{
				if (!PropertyData.TargetObject.IsValid())
				{
					return true;
				}
				return (PropertyData.TargetObject->GetWorld() == World);
			});

		RegisterPendingFunctions.RemoveAll([&](const FGDMPendingObjectData& FunctionData)
			{
				if (!FunctionData.TargetObject.IsValid())
				{
					return true;
				}
				return (FunctionData.TargetObject->GetWorld() == World);
			});
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(PlayerController, false);
	if(IsValid(GDMManager))
	{
		GDMManager->HideDebugMenu();
		GDMManager->Destroy();
		return true;
	}

	return false;
}

AGameDebugMenuManager* UGameDebugMenuFunctions::GetGameDebugMenuManager(const UObject* WorldContextObject, const bool bCheckInitialize)
{
	if(bDisableGameDebugMenu)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if( !IsValid(World) )
	{
		UE_LOG(LogGDM, Warning, TEXT("GetGameDebugMenuManager: not found World"));
		return nullptr;
	}

	AGameDebugMenuManager* GDMManager = CurrentGameDebugMenuManager.Get();
	if (IsValid(GDMManager))
	{
		if (GDMManager->GetWorld() == World)
		{
			if(bCheckInitialize)
			{
				return GDMManager->IsInitializedManager() ? GDMManager : nullptr;
			}

			return GDMManager;
		}
	}

	/* 違うワールドの場合は検索して取得し直す（主にエディターなど） */
	GDMManager = nullptr;

	for (const auto Manager : GGameDebugMenuManagers )
	{
		if (Manager->GetWorld() == World)
		{
			GDMManager = Cast<AGameDebugMenuManager>(Manager);
			break;
		}
	}

	if (IsValid(GDMManager))
	{
		if(bCheckInitialize)
		{
			return GDMManager->IsInitializedManager() ? GDMManager : nullptr;
		}

		return GDMManager;
	}

	return nullptr;
}

bool UGameDebugMenuFunctions::ShowDebugMenu(UObject* WorldContextObject)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("ShowDebugMenu: GameDebugMenu is disabled"));
		return false;
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Warning, TEXT("ShowDebugMenu: Instance Not Found"));
		return false;
	}

	return GDMManager->ShowDebugMenu();
}


bool UGameDebugMenuFunctions::HideDebugMenu(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Log, TEXT("HideDebugMenu: Instance Not Found"));
		return false;
	}

	GDMManager->HideDebugMenu();

	return true;
}

bool UGameDebugMenuFunctions::ToggleDebugMenu(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Log, TEXT("ToggleDebugMenu: Instance Not Found"));
		return false;
	}

	if(GDMManager->IsShowingDebugMenu())
	{
		GDMManager->HideDebugMenu();
	}
	else
	{
		GDMManager->ShowDebugMenu();
	}
	return true;
}

bool UGameDebugMenuFunctions::IsShowingDebugMenu(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return false;
	}

	return GDMManager->IsShowingDebugMenu();
}

UGameDebugMenuRootWidget* UGameDebugMenuFunctions::GetGameDebugMenuRootWidget(const UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}

	return GDMManager->GetDebugMenuRootWidget();
}

bool UGameDebugMenuFunctions::RegisterGDMObjectProperty(UObject* TargetObject, const FGDMPropertyUIConfigInfo PropertyUIConfigInfo, const FName PropertyName, const FGDMGameplayCategoryKey CategoryKey, const FString PropertySaveKey, const FText DisplayPropertyName,const FText Description, const int32 DisplayPriority)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterGDMObjectProperty: GameDebugMenu is disabled"));
		return false;
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		/* まだ生成してないので一時キャッシュ */
		FGDMPendingObjectData PendingData;
		PendingData.TargetObject        = TargetObject;
		PendingData.TargetName          = PropertyName;
		PendingData.CategoryKey			= CategoryKey;
		PendingData.SaveKey				= PropertySaveKey;
		PendingData.DisplayPropertyName = DisplayPropertyName;
		PendingData.Description         = Description;
		PendingData.ConfigInfo          = PropertyUIConfigInfo;
		PendingData.DisplayPriority		= DisplayPriority;
		RegisterPendingProperties.Add(PendingData);
		return false;
	}
	
	return GDMManager->RegisterObjectProperty(TargetObject, PropertyName, CategoryKey, PropertySaveKey, DisplayPropertyName, Description, PropertyUIConfigInfo, DisplayPriority);
}

bool UGameDebugMenuFunctions::RegisterGDMObjectFunction(UObject* TargetObject, FName FunctionName, const FGDMGameplayCategoryKey CategoryKey, const FString FunctionSaveKey, const FText DisplayFunctionName,const FText Description, const int32 DisplayPriority)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterGDMObjectFunction: GameDebugMenu is disabled"));
		return false;
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		/* まだ生成してないので一時キャッシュ */
		FGDMPendingObjectData PendingData;
		PendingData.TargetObject        = TargetObject;
		PendingData.TargetName          = FunctionName;
		PendingData.CategoryKey			= CategoryKey;
		PendingData.SaveKey				= FunctionSaveKey;
		PendingData.DisplayPropertyName = DisplayFunctionName;
		PendingData.Description         = Description;
		PendingData.DisplayPriority		= DisplayPriority;
		RegisterPendingFunctions.Add(PendingData);
		return false;
	}
	
	return GDMManager->RegisterObjectFunction(TargetObject, FunctionName, CategoryKey, FunctionSaveKey, DisplayFunctionName, Description, DisplayPriority);
}

void UGameDebugMenuFunctions::UnregisterGDMObject(UObject* TargetObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		return;
	}

	FGDMGameplayCategoryKey CategoryKey;
	FString PropertySaveKey;
	FText DisplayPropertyName;
	FText Description;
	FName PropertyName;
	EGDMPropertyType PropertyType;
	FString EnumPathName;
	FGDMPropertyUIConfigInfo PropertyUIConfigInfo;

	for(int32 Index = GDMManager->GetNumObjectProperties() - 1; Index >= 0; --Index)
	{
		UObject* RegisterObject = GDMManager->GetObjectProperty(Index, CategoryKey, PropertySaveKey, DisplayPropertyName, Description, PropertyName, PropertyType, EnumPathName, PropertyUIConfigInfo);
		if(!IsValid(RegisterObject) || (TargetObject == RegisterObject))
		{
			GDMManager->RemoveObjectProperty(Index);
		}
	}

	FText DisplayFunctionName;
	FName FunctionName;
	for(int32 Index = GDMManager->GetNumObjectFunctions() - 1; Index >= 0; --Index)
	{
		UObject* RegisterObject = GDMManager->GetObjectFunction(Index, CategoryKey, PropertySaveKey, DisplayFunctionName, Description, FunctionName);
		if(!IsValid(RegisterObject) || (TargetObject == RegisterObject))
		{
			GDMManager->RemoveObjectFunction(Index);
		}
	}
}

UObject* UGameDebugMenuFunctions::GetGDMObjectProperty(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutPropertySaveKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FString& OutEnumPathName, FGDMPropertyUIConfigInfo& PropertyUIConfigInfo)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}
	return GDMManager->GetObjectProperty(Index, OutCategoryKey, OutPropertySaveKey, OutDisplayPropertyName, OutDescription, OutPropertyName, OutPropertyType, OutEnumPathName, PropertyUIConfigInfo);
}

UObject* UGameDebugMenuFunctions::GetGDMObjectFunction(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FString& OutFunctionSaveKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}
	return GDMManager->GetObjectFunction(Index, OutCategoryKey, OutFunctionSaveKey, OutDisplayFunctionName, OutDescription, OutFunctionName);
}

int32 UGameDebugMenuFunctions::GetGDMNumObjectProperties(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return 0;
	}
	return GDMManager->GetNumObjectProperties();
}

int32 UGameDebugMenuFunctions::GetGDMNumObjectFunctions(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return 0;
	}
	return GDMManager->GetNumObjectFunctions();
}

bool UGameDebugMenuFunctions::VerifyGDMNumObjectProperties(UObject* WorldContextObject)
{
	UE_LOG(LogGDM,Verbose, TEXT("VerifyGDMNumObjectProperties"));

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return false;
	}

	FGDMGameplayCategoryKey CategoryKey;
	FString PropertySaveKey;
	FText DisplayPropertyName;
	FText Description;
	FName PropertyName;
	EGDMPropertyType PropertyType;
	FString EnumPathName;
	FGDMPropertyUIConfigInfo PropertyUIConfigInfo;
	int32 RemoveCount = 0;

	for(int32 Index = GetGDMNumObjectProperties(WorldContextObject) - 1; Index >= 0; --Index)
	{
		const UObject* PropertyOwnerObj = GetGDMObjectProperty(WorldContextObject, Index, CategoryKey, PropertySaveKey, DisplayPropertyName, Description, PropertyName, PropertyType, EnumPathName, PropertyUIConfigInfo);
		if(!IsValid(PropertyOwnerObj))
		{
			GDMManager->RemoveObjectProperty(Index);
			RemoveCount++;
		}
	}

	UE_LOG(LogGDM,Verbose, TEXT("VerifyGDMNumObjectProperties: Remove %d"), RemoveCount);

	return (RemoveCount != 0);
}

bool UGameDebugMenuFunctions::VerifyGDMNumObjectFunctions(UObject* WorldContextObject)
{
	UE_LOG(LogGDM,Verbose, TEXT("VerifyGDMNumObjectFunctions"));

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return false;
	}

	const int32 Num = GetGDMNumObjectFunctions(WorldContextObject);

	FGDMGameplayCategoryKey CategoryKey;
	FString FunctionSaveKey;
	FText DisplayFunctionName;
	FText Description;
	FName FunctionName;
	int32 RemoveCount = 0;

	for(int32 Index = Num - 1; Index >= 0; --Index)
	{
		const UObject* FunctionOwnerObj = GetGDMObjectFunction(WorldContextObject,Index, CategoryKey,FunctionSaveKey, DisplayFunctionName,Description,FunctionName);
		if(!IsValid(FunctionOwnerObj))
		{
			GDMManager->RemoveObjectFunction(Index);
			RemoveCount++;
		}
	}

	UE_LOG(LogGDM,Verbose, TEXT("VerifyGDMNumObjectFunctions: Remove %d"), RemoveCount);
	
	return (RemoveCount == 0);
}

TArray<FGDMMenuCategoryKey> UGameDebugMenuFunctions::GetGDMOrderGameplayCategoryTitle(UObject* WorldContextObject)
{
	TArray<FGDMMenuCategoryKey> ReturnValues;
	const auto& TitleArray = GetDefault<UGameDebugMenuSettings>()->OrderGameplayCategoryTitles;
	for( int32 Index = 0; Index < TitleArray.Num(); ++Index )
	{
		ReturnValues.Add(FGDMMenuCategoryKey(TitleArray[Index].Index, TitleArray[Index].Title));
	}

	return ReturnValues;
}

int32 UGameDebugMenuFunctions::GetGDMInvalidIndex()
{
	return INDEX_NONE;
}

FString UGameDebugMenuFunctions::GetGDMObjectName(UObject* TargetObject)
{
	if(!IsValid(TargetObject))
	{
		return FString();
	}

#if WITH_EDITOR
	return UKismetSystemLibrary::GetDisplayName(TargetObject);
#else
	return UKismetSystemLibrary::GetObjectName(TargetObject);
#endif
}

void UGameDebugMenuFunctions::RegisterInputComponentForGameDebugMenu(UObject* WorldContextObject, UInputComponent* InputComponent)
{
	if (AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject))
	{
		GDMManager->GetDebugMenuInputSystemComponent()->RegisterInputComponent(InputComponent);
	}
}

void UGameDebugMenuFunctions::UnregisterInputComponentForGameDebugMenu(UObject* WorldContextObject, UInputComponent* InputComponent)
{
	if (AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject))
	{
		GDMManager->GetDebugMenuInputSystemComponent()->UnregisterInputComponent(InputComponent);
	}
}

bool UGameDebugMenuFunctions::ShowDebugReport(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Log, TEXT("RequestDebugReport: Instance not found"));
		return false;
	}

	if(GDMManager->ShowDebugMenu(true))
	{
		return true;
	}

	return false;
}

EGDMProjectManagementTool UGameDebugMenuFunctions::GetGDMSelectedProjectManagementTool()
{
	return GetDefault<UGameDebugMenuSettings>()->ProjectManagementToolType;
}

FString UGameDebugMenuFunctions::GetGDMBuildConfigurationString()
{
#if UE_EDITOR
	return TEXT("Editor");
#else
	return LexToString(FApp::GetBuildConfiguration());
#endif
}

FString UGameDebugMenuFunctions::GetGDMBuildVersionString()
{
	return FApp::GetBuildVersion();
}

FString UGameDebugMenuFunctions::GetGDMProjectVersionString()
{
	const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();
	return *ProjectSettings.ProjectVersion;
}

TArray<FString> UGameDebugMenuFunctions::GetGDMCultureList()
{
	return GetDefault<UGameDebugMenuSettings>()->CultureList;
}

void UGameDebugMenuFunctions::PrintLogScreen(UObject* WorldContextObject, const FString& InString, float Duration, bool bPrintToLog)
{
#if !(UE_BUILD_SHIPPING)

	static FLinearColor LogColor(0.15f, 0.2f, 0.2f);
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client %d: "), UE::GetPlayInEditorID() - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server: "));
				break;
			case NM_Standalone:
				break;
			default: ;
			}
		}
	}

	const FString FinalDisplayString = Prefix + InString;
	FString FinalLogString = FinalDisplayString;

	static const FBoolConfigValueHelper DisplayPrintStringSource(TEXT("Kismet"), TEXT("bLogPrintStringSource"), GEngineIni);
	if (DisplayPrintStringSource)
	{
		const FString SourceObjectPrefix = FString::Printf(TEXT("[%s] "), *GetNameSafe(WorldContextObject));
		FinalLogString = SourceObjectPrefix + FinalLogString;
	}

	if (GAreScreenMessagesEnabled)
	{
		if (GConfig && Duration < 0)
		{
			GConfig->GetFloat(TEXT("Kismet"), TEXT("PrintStringDuration"), Duration, GEngineIni);
		}
		GEngine->AddOnScreenDebugMessage((uint64)-1, Duration, LogColor.ToFColor(true), TEXT("GDM: ") + FinalDisplayString);
	}
	else
	{
		UE_LOG(LogGDM, Log, TEXT("Screen messages disabled (!GAreScreenMessagesEnabled).  Cannot print to screen."));
	}

	if( bPrintToLog )
	{
		UE_LOG(LogGDM, Log, TEXT("%s"), *FinalDisplayString);
	}
	else
	{
		UE_LOG(LogGDM, Verbose, TEXT("%s"), *FinalLogString);
	}

#endif
}

bool UGameDebugMenuFunctions::GetDebugMenuString(UObject* WorldContextObject, const FString StringKey, FString& OutString)
{
	if(const AGameDebugMenuManager* Manager = GetGameDebugMenuManager(WorldContextObject) )
	{
		return Manager->GetLocalizeStringComponent()->GetString(StringKey, OutString);
	}

	OutString = StringKey;
	return false;
}

FName UGameDebugMenuFunctions::GetCurrentDebugMenuLanguage(UObject* WorldContextObject)
{
	if(const AGameDebugMenuManager* Manager = GetGameDebugMenuManager(WorldContextObject) )
	{
		return Manager->GetLocalizeStringComponent()->GetCurrentDebugMenuLanguage();
	}

	return NAME_None;
}

TArray<FName> UGameDebugMenuFunctions::GetDebugMenuLanguageKeys()
{
	return GetDefault<UGameDebugMenuSettings>()->GetDebugMenuLanguageKeys();
}

FString UGameDebugMenuFunctions::GetDebugMenuLineBreakString()
{
	return GetDefault<UGameDebugMenuSettings>()->LineBreakString;
}

FString UGameDebugMenuFunctions::BuildConsoleCommandId_FromSingle(const FGDMConsoleCommandSingle& Command)
{
	return Command.BuildCommandIdentifier();
}

FString UGameDebugMenuFunctions::BuildConsoleCommandId_FromGroup(const FGDMConsoleCommandGroup& Command)
{
	return Command.BuildCommandIdentifier();
}

FString UGameDebugMenuFunctions::BuildConsoleCommandId_FromPair(const FGDMConsoleCommandPair& Command)
{
	return Command.BuildCommandIdentifier();
}

FString UGameDebugMenuFunctions::BuildConsoleCommandId_FromNumber(const FGDMConsoleCommandNumber& Command)
{
	return Command.BuildCommandIdentifier();
}


bool UGameDebugMenuFunctions::EqualEqual_GDMMenuCategoryKey(const FGDMMenuCategoryKey& A, const FGDMMenuCategoryKey& B)
{
	return A.Index == B.Index;
}

bool UGameDebugMenuFunctions::NotEqual_GDMMenuCategoryKey(const FGDMMenuCategoryKey& A, const FGDMMenuCategoryKey& B)
{
	return A.Index != B.Index;
}

uint8 UGameDebugMenuFunctions::Conv_GDMMenuCategoryKeyToByte(const FGDMMenuCategoryKey& Key)
{
	return Key.Index;
}

FGDMMenuCategoryKey UGameDebugMenuFunctions::Conv_ByteToGDMMenuCategoryKey(const uint8& Index)
{
	return FGDMMenuCategoryKey(Index);
}

void UGameDebugMenuFunctions::OnActorSpawnedClientWaitManager(AGameDebugMenuManager* SpawnDebugMenuManager)
{	
	CurrentGameDebugMenuManager = SpawnDebugMenuManager;

	/* マネージャー生成前に登録処理したプロパティ群を追加する */
	for(const auto& PendingData : RegisterPendingProperties)
	{
		if(PendingData.TargetObject.IsValid())
		{
			if( PendingData.TargetObject->GetWorld() == SpawnDebugMenuManager->GetWorld() )
			{
				RegisterGDMObjectProperty(PendingData.TargetObject.Get()
					,PendingData.ConfigInfo
					,PendingData.TargetName
					,PendingData.CategoryKey
					,PendingData.SaveKey
					,PendingData.DisplayPropertyName
					,PendingData.Description
					,PendingData.DisplayPriority
				);
			}
		}
	}

	/* マネージャー生成前に登録処理した関数群を追加する */
	for(const auto& PendingData : RegisterPendingFunctions)
	{
		if(PendingData.TargetObject.IsValid())
		{
			if( PendingData.TargetObject->GetWorld() == SpawnDebugMenuManager->GetWorld() )
			{
				RegisterGDMObjectFunction(PendingData.TargetObject.Get()
					,PendingData.TargetName
					,PendingData.CategoryKey
					,PendingData.SaveKey
					,PendingData.DisplayPropertyName
					,PendingData.Description
					,PendingData.DisplayPriority
				);
			}
		}
	}

	RegisterPendingProperties.RemoveAll([&](const FGDMPendingObjectData& PropertyData)
		{
			if (!PropertyData.TargetObject.IsValid())
			{
				return true;
			}
			return (PropertyData.TargetObject->GetWorld() == SpawnDebugMenuManager->GetWorld());
		});

	RegisterPendingFunctions.RemoveAll([&](const FGDMPendingObjectData& FunctionData)
		{
			if (!FunctionData.TargetObject.IsValid())
			{
				return true;
			}
			return (FunctionData.TargetObject->GetWorld() == SpawnDebugMenuManager->GetWorld());
		});

	UE_LOG(LogGDM, Log, TEXT("OnActorSpawnedClientWaitManager: Spawn GameDebugMenuManager"));
}

void UGameDebugMenuFunctions::OnActorSpawnedServer(AActor* SpawnActor)
{
	if (!IsValid(SpawnActor))
	{
		return;
	}

	if (!SpawnActor->IsA(APlayerController::StaticClass()))
	{
		return;
	}

	AGameDebugMenuManager* DebugMenuManager = GetGameDebugMenuManager(SpawnActor);
	if (!IsValid(DebugMenuManager))
	{
		UE_LOG(LogGDM, Warning, TEXT("OnActorSpawnedServer: Not found DebugMenuManager"));
		return;
	}

	DebugMenuManager->AddDebugMenuPCProxyComponent(Cast<APlayerController>(SpawnActor));
}

void UGameDebugMenuFunctions::ShowDebugConsoleCommand()
{
	/* キャッシュしてるマネージャーに対して実行 */
	if( CurrentGameDebugMenuManager.IsValid())
	{
		CurrentGameDebugMenuManager->ShowDebugMenu();
	}
}

void UGameDebugMenuFunctions::HideDebugConsoleCommand()
{
	/* キャッシュしてるマネージャーに対して実行 */
	if( CurrentGameDebugMenuManager.IsValid())
	{
		CurrentGameDebugMenuManager->HideDebugMenu();
	}
}

void UGameDebugMenuFunctions::ToggleDebugConsoleCommand()
{
	/* キャッシュしてるマネージャーに対して実行 */
	if( CurrentGameDebugMenuManager.IsValid())
	{
		if( CurrentGameDebugMenuManager->IsShowingDebugMenu())
		{
			CurrentGameDebugMenuManager->HideDebugMenu();
		}
		else
		{
			CurrentGameDebugMenuManager->ShowDebugMenu();
		}
	}
}

void UGameDebugMenuFunctions::ToggleInputSystemLog()
{
	/* キャッシュしてるマネージャーに対して実行 */
	if( CurrentGameDebugMenuManager.IsValid() )
	{
		UGDMInputSystemComponent* InputSystemComponent = CurrentGameDebugMenuManager->GetDebugMenuInputSystemComponent();
		InputSystemComponent->bOutputDebugLog = !InputSystemComponent->bOutputDebugLog;
	}
}
