/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuFunctions.h"
#include "GameDebugMenu.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameDebugMenuSettings.h"
#include "GDMPlayerControllerProxyComponent.h"
#include "CoreGlobals.h"
#include "Misc/ConfigCacheIni.h"
#include <GeneralProjectSettings.h>
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

void UGameDebugMenuFunctions::DisableGameDebugMenu() 
{
	bDisableGameDebugMenu = true; 
	RegisterPendingProperties.Empty();
	RegisterPendingFunctions.Empty();
};

bool UGameDebugMenuFunctions::TryCreateDebugMenuManager(UObject* WorldContextObject, TSoftClassPtr<AGameDebugMenuManager> DebugMenuManagerClassPtr)
{
	bDisableGameDebugMenu = false;
	bool bServer    = UKismetSystemLibrary::IsServer(WorldContextObject);
	FString NetMode = (bServer ? TEXT("Server") : TEXT("Client"));

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if(World == nullptr)
	{
		UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Not found world[%s]"), *NetMode);
		return false;
	}

	if(!bServer)
	{
		/* Client */
		if( !IsValid(GetGameDebugMenuManager(WorldContextObject)) )
		{
			/* まだ生成してもらってないので待つ */
			UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Wait spawn[%s]"), *NetMode);
			return false;
		}

		CurrentGameDebugMenuManager = GetGameDebugMenuManager(WorldContextObject);
	}
	else
	{
		/* Server */
		if (IsValid(GetGameDebugMenuManager(WorldContextObject)))
		{
			UE_LOG(LogGDM, Log, TEXT("TryCreateDebugMenuManager: Created manager[%s]"), *NetMode);
			return true;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		UClass* DebugMenuManagerClass = DebugMenuManagerClassPtr.LoadSynchronous();
		if(DebugMenuManagerClass == nullptr)
		{
			FString ErrorStr = TEXT("DebugMenuManagerClass NotFound LoadError");
			ErrorStr += TEXT("[");
			ErrorStr += NetMode;
			ErrorStr += TEXT("]");

			UKismetSystemLibrary::PrintString(WorldContextObject, ErrorStr);
			return false;
		}

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
				RegisterGDMObjectProperty(PendingData.TargetObject.Get(),
										  PendingData.ConfigInfo,
										  PendingData.TargetName,
										  PendingData.CategoryKey,
										  PendingData.DisplayPropertyName,
										  PendingData.Description,
										  PendingData.DisplayPriority
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

bool UGameDebugMenuFunctions::DestroyDebugMenuManager(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
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

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(IsValid(GDMManager))
	{
		GDMManager->HideDebugMenu();
		GDMManager->Destroy();
		return true;
	}

	return false;
}

AGameDebugMenuManager* UGameDebugMenuFunctions::GetGameDebugMenuManager(UObject* WorldContextObject)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("GetGameDebugMenuManager: GameDebugMenu is disabled"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
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
			return GDMManager;
		}
	}

	/* 違うワールドの場合は検索して取得し直す（主にエディターなど） */
	GDMManager = nullptr;

	for (auto Manager : GGameDebugMenuManagers )
	{
		if (Manager->GetWorld() == World)
		{
			GDMManager = Cast<AGameDebugMenuManager>(Manager);
			break;
		}
	}

	return GDMManager;
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

UGameDebugMenuRootWidget* UGameDebugMenuFunctions::GetGameDebugMenuRootWidget(UObject* WorldContextObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}

	return GDMManager->GetDebugMenuRootWidget();
}

bool UGameDebugMenuFunctions::RegisterGDMObjectProperty(UObject* TargetObject, const FGDMPropertyUIConfigInfo PropertyUIConfigInfo,FName PropertyName,const FGDMGameplayCategoryKey CategoryKey,const FText DisplayPropertyName,const FText Description, const int32 DisplayPriority)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterGDMObjectProperty: GameDebugMenu is disabled"));
		return false;
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		FGDMPendingObjectData PendingData;
		PendingData.TargetObject        = TargetObject;
		PendingData.TargetName          = PropertyName;
		PendingData.CategoryKey			= CategoryKey;
		PendingData.DisplayPropertyName = DisplayPropertyName;
		PendingData.Description         = Description;
		PendingData.ConfigInfo          = PropertyUIConfigInfo;
		PendingData.DisplayPriority		= DisplayPriority;
		RegisterPendingProperties.Add(PendingData);
		return false;
	}
	return GDMManager->RegisterObjectProperty( TargetObject, PropertyName, CategoryKey, DisplayPropertyName, Description, PropertyUIConfigInfo, DisplayPriority);
}

bool UGameDebugMenuFunctions::RegisterGDMObjectFunction(UObject* TargetObject,FName FunctionName,const FGDMGameplayCategoryKey CategoryKey,const FText DisplayFunctionName,const FText Description, const int32 DisplayPriority)
{
	if(bDisableGameDebugMenu)
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterGDMObjectFunction: GameDebugMenu is disabled"));
		return false;
	}

	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		FGDMPendingObjectData PendingData;
		PendingData.TargetObject        = TargetObject;
		PendingData.TargetName          = FunctionName;
		PendingData.CategoryKey			= CategoryKey;
		PendingData.DisplayPropertyName = DisplayFunctionName;
		PendingData.Description         = Description;
		PendingData.DisplayPriority		= DisplayPriority;
		RegisterPendingFunctions.Add(PendingData);
		return false;
	}
	return GDMManager->RegisterObjectFunction(TargetObject,FunctionName, CategoryKey, DisplayFunctionName, Description, DisplayPriority);
}

void UGameDebugMenuFunctions::UnregisterGDMObject(UObject* TargetObject)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(TargetObject);
	if(!IsValid(GDMManager))
	{
		return;
	}

	FGDMGameplayCategoryKey CategoryKey;
	FText DisplayPropertyName;
	FText Description;
	FName PropertyName;
	EGDMPropertyType PropertyType;
	FName EnumTypeName;
	FGDMPropertyUIConfigInfo PropertyUIConfigInfo;

	for(int32 Index = GDMManager->GetNumObjectProperties() - 1; Index >= 0; --Index)
	{
		UObject* RegisterObject = GDMManager->GetObjectProperty(Index, CategoryKey, DisplayPropertyName, Description, PropertyName, PropertyType, EnumTypeName, PropertyUIConfigInfo);
		if(!IsValid(RegisterObject) || (TargetObject == RegisterObject))
		{
			GDMManager->RemoveObjectProperty(Index);
		}
	}

	FText DisplayFunctionName;
	FName FunctionName;
	for(int32 Index = GDMManager->GetNumObjectFunctions() - 1; Index >= 0; --Index)
	{
		UObject* RegisterObject = GDMManager->GetObjectFunction(Index, CategoryKey, DisplayFunctionName, Description, FunctionName);
		if(!IsValid(RegisterObject) || (TargetObject == RegisterObject))
		{
			GDMManager->RemoveObjectFunction(Index);
		}
	}
}

UObject* UGameDebugMenuFunctions::GetGDMObjectProperty(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayPropertyName, FText& OutDescription, FName& OutPropertyName, EGDMPropertyType& OutPropertyType, FName& OutEnumTypeName, FGDMPropertyUIConfigInfo& PropertyUIConfigInfo)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}
	return GDMManager->GetObjectProperty(Index, OutCategoryKey, OutDisplayPropertyName, OutDescription, OutPropertyName, OutPropertyType, OutEnumTypeName, PropertyUIConfigInfo);
}

UObject* UGameDebugMenuFunctions::GetGDMObjectFunction(UObject* WorldContextObject,const int32 Index, FGDMGameplayCategoryKey& OutCategoryKey, FText& OutDisplayFunctionName, FText& OutDescription, FName& OutFunctionName)
{
	AGameDebugMenuManager* GDMManager = GetGameDebugMenuManager(WorldContextObject);
	if(!IsValid(GDMManager))
	{
		return nullptr;
	}
	return GDMManager->GetObjectFunction(Index, OutCategoryKey, OutDisplayFunctionName, OutDescription, OutFunctionName);
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
	FText DisplayPropertyName;
	FText Description;
	FName PropertyName;
	EGDMPropertyType PropertyType;
	FName EnumTypeName;
	FGDMPropertyUIConfigInfo PropertyUIConfigInfo;
	int32 RemoveCount = 0;

	for(int32 Index = GetGDMNumObjectProperties(WorldContextObject) - 1; Index >= 0; --Index)
	{
		UObject* PropertyOwnerObj = GetGDMObjectProperty(WorldContextObject,Index, CategoryKey, DisplayPropertyName, Description, PropertyName, PropertyType, EnumTypeName, PropertyUIConfigInfo);
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

	int32 Num = GetGDMNumObjectFunctions(WorldContextObject);

	FGDMGameplayCategoryKey CategoryKey;
	FText DisplayFunctionName;
	FText Description;
	FName FunctionName;
	int32 RemoveCount = 0;

	for(int32 Index = Num - 1; Index >= 0; --Index)
	{
		UObject* FunctionOwnerObj = GetGDMObjectFunction(WorldContextObject,Index,CategoryKey,DisplayFunctionName,Description,FunctionName);
		if(!IsValid(FunctionOwnerObj))
		{
			GDMManager->RemoveObjectFunction(Index);
			RemoveCount++;
		}
	}

	UE_LOG(LogGDM,Verbose, TEXT("VerifyGDMNumObjectFunctions: Remove %d"), RemoveCount);
	
	return (RemoveCount == 0);
}

bool UGameDebugMenuFunctions::GetGDMConsoleCommandNameByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandSingle& Out)
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandSingle> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandNames);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandNames);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = UGameDebugMenuSettings::Get()->ConsoleCommandNames;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGameDebugMenuFunctions::GetGDMNumConsoleCommandNames()
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandSingle> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandNames);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandNames);
	return Commands.Num();
#else
	return UGameDebugMenuSettings::Get()->ConsoleCommandNames.Num();
#endif
}

bool UGameDebugMenuFunctions::GetGDMConsoleCommandGroupByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandGroup& Out)
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandGroup> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandGroups);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandGroups);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = UGameDebugMenuSettings::Get()->ConsoleCommandGroups;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGameDebugMenuFunctions::GetGDMNumConsoleCommandGroups()
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandGroup> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandGroups);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandGroups);
	return Commands.Num();
#else
	return UGameDebugMenuSettings::Get()->ConsoleCommandGroups.Num();
#endif
}

bool UGameDebugMenuFunctions::GetGDMConsoleCommandPairByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandPair& Out)
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandPair> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandPairs);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandPairs);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = UGameDebugMenuSettings::Get()->ConsoleCommandPairs;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGameDebugMenuFunctions::GetGDMNumConsoleCommandPairs()
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandPair> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandPairs);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandPairs);
	return Commands.Num();
#else
	return UGameDebugMenuSettings::Get()->ConsoleCommandPairs.Num();
#endif
}

bool UGameDebugMenuFunctions::GetGDMConsoleCommandNumberByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandNumber& Out)
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandNumber> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandNumbers);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandNumbers);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = UGameDebugMenuSettings::Get()->ConsoleCommandNumbers;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGameDebugMenuFunctions::GetGDMNumConsoleCommandNumbers()
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandNumber> Commands;
	Commands.Append(UGameDebugMenuSettings::Get()->ConsoleCommandNumbers);
	Commands.Append(UGameDebugMenuSettings::Get()->EditorOnlyConsoleCommandNumbers);
	return Commands.Num();
#else
	return UGameDebugMenuSettings::Get()->ConsoleCommandNumbers.Num();
#endif
}

TArray<FGDMMenuCategoryKey> UGameDebugMenuFunctions::GetOrderConsoleCommandCategoryTitle(UObject* WorldContextObject)
{
	TArray<FGDMMenuCategoryKey> ReturnValues;
	const auto& TitleArray = UGameDebugMenuSettings::Get()->OrderConsoleCommandCategoryTitles;
	for( int32 Index = 0;Index < TitleArray.Num();++Index )
	{
		ReturnValues.Add(FGDMMenuCategoryKey(TitleArray[Index].Index, TitleArray[Index].Title));
	}

	return ReturnValues;
}

TArray<FGDMMenuCategoryKey> UGameDebugMenuFunctions::GetOrderGameplayCategoryTitle(UObject* WorldContextObject)
{
	TArray<FGDMMenuCategoryKey> ReturnValues;
	const auto& TitleArray = UGameDebugMenuSettings::Get()->OrderGameplayCategoryTitles;
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

bool UGameDebugMenuFunctions::RegisterGDMInputObject(UObject* TargetInputObject)
{
	AGameDebugMenuManager* GDMManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(TargetInputObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterGDMInputObject: Not found DebugMenuManager"));
		return false;
	}

	return GDMManager->RegisterInputObject(TargetInputObject);
}

bool UGameDebugMenuFunctions::UnregisterGDMInputObject(UObject* TargetInputObject)
{
	AGameDebugMenuManager* GDMManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(TargetInputObject);
	if(!IsValid(GDMManager))
	{
		UE_LOG(LogGDM, Warning, TEXT("UnregisterGDMInputObject: Not found DebugMenuManager"));
		return false;
	}

	return GDMManager->UnregisterInputObject(TargetInputObject);
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

EGDMProjectManagementTool UGameDebugMenuFunctions::GetSelectedProjectManagementTool()
{
	return UGameDebugMenuSettings::Get()->ProjectManagementToolType;
}

FString UGameDebugMenuFunctions::GetBuildConfigurationString()
{
#if UE_EDITOR
	return TEXT("Editor");
#else
	return LexToString(FApp::GetBuildConfiguration());
#endif
}

FString UGameDebugMenuFunctions::GetBuildVersionString()
{
	return FApp::GetBuildVersion();
}

FString UGameDebugMenuFunctions::GetProjectVersionString()
{
	const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();
	return *ProjectSettings.ProjectVersion;
}

float UGameDebugMenuFunctions::GetWorldTimeDilation(UObject* WorldContextObject)
{
	if( UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::ReturnNull) )
	{
		return World->GetWorldSettings()->TimeDilation;
	}

	return 1.0f;
}

TArray<FString> UGameDebugMenuFunctions::GetGDMCultureList()
{
	return UGameDebugMenuSettings::Get()->CultureList;
}

void UGameDebugMenuFunctions::PrintLogScreen(UObject* WorldContextObject, const FString& InString, float Duration, bool bPrintToLog)
{
#if !(UE_BUILD_SHIPPING)

	static FLinearColor LogColor(0.15f, 0.2f, 0.2f);
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client %d: "), GPlayInEditorID - 1);
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server: "));
				break;
			case NM_Standalone:
				break;
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

void UGameDebugMenuFunctions::DynamicallyChangeBlockInput(UObject* WorldContextObject, bool bBlockFlag)
{
	if(AGameDebugMenuManager* Manager = GetGameDebugMenuManager(WorldContextObject))
	{
		if(IsValid(Manager->InputComponent))
		{
			Manager->InputComponent->bBlockInput = bBlockFlag;
		}
	}
}

bool UGameDebugMenuFunctions::GetDebugMenuString(UObject* WorldContextObject, const FString StringKey, FString& OutString)
{
	if( AGameDebugMenuManager* Manager = GetGameDebugMenuManager(WorldContextObject) )
	{
		return Manager->GetDebugMenuString(StringKey, OutString);
	}

	OutString = StringKey;
	return false;
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
				RegisterGDMObjectProperty(PendingData.TargetObject.Get(),
										  PendingData.ConfigInfo,
										  PendingData.TargetName,
										  PendingData.CategoryKey,
										  PendingData.DisplayPropertyName,
										  PendingData.Description,
										  PendingData.DisplayPriority
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
				RegisterGDMObjectFunction(PendingData.TargetObject.Get(),
										  PendingData.TargetName,
										  PendingData.CategoryKey,
										  PendingData.DisplayPropertyName,
										  PendingData.Description,
										  PendingData.DisplayPriority
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

	AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(SpawnActor);
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
		if( UGDMInputSystemComponent* InputSystemComponent = CurrentGameDebugMenuManager->GetDebugMenuInputSystemComponent() )
		{
			InputSystemComponent->bOutputDebugLog = !InputSystemComponent->bOutputDebugLog;
		}
	}
}
