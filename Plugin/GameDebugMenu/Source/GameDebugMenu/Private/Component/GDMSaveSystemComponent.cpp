/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMSaveSystemComponent.h"

#include "GameDebugMenuManager.h"
#include "GameDebugMenuSettings.h"
#include "Component/GDMPropertyJsonSystemComponent.h"
#include "Kismet/GameplayStatics.h"

UGDMSaveSystemComponent::UGDMSaveSystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, UserIndex(0)
	, SaveGame(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGDMSaveSystemComponent::SaveDebugMenuFile()
{
	if (GetDefault<UGameDebugMenuSettings>()->bDisableSaveFile)
	{
		return;
	}
	
	AGameDebugMenuManager* Manager = Cast<AGameDebugMenuManager>(GetOwner());
	if (!IsValid(Manager))
	{
		UE_LOG(LogGDM, Error, TEXT("SaveDebugMenuFile: GameDebugMenuManager not found on owner actor."));
		return;
	}
	
	UGDMPropertyJsonSystemComponent* JsonSystemComponent = GetPropertyJsonSystemComponent();
	if (!IsValid(JsonSystemComponent))
	{
		UE_LOG(LogGDM, Error, TEXT("SaveDebugMenuFile: PropertyJsonSystemComponent not found on the same actor."));
		return;
	}

	TArray<FString> CommandHistory;
	if (!GetDefault<UGameDebugMenuSettings>()->bDoesNotSaveConsoleCommand)
	{
		Manager->GetOutputCommandHistoryString(CommandHistory);
	}
	
	JsonSystemComponent->SetCustomStringArray(TEXT("CommandHistory"), CommandHistory);
	
	const FString JsonString = JsonSystemComponent->GetJsonAsString();
	if (JsonString.IsEmpty())
	{
		UE_LOG(LogGDM, Log, TEXT("SaveDebugMenuFile: JsonString is empty."));
		return;
	}

	if (SaveFile(JsonString))
	{
		Manager->CallSavedDebugMenuDispatcher();
	}
}

void UGDMSaveSystemComponent::LoadDebugMenuFile()
{
	if (GetDefault<UGameDebugMenuSettings>()->bDisableSaveFile)
	{
		return;
	}

	AGameDebugMenuManager* Manager = Cast<AGameDebugMenuManager>(GetOwner());
	if (!IsValid(Manager))
	{
		UE_LOG(LogGDM, Error, TEXT("LoadDebugMenuFile: GameDebugMenuManager not found on owner actor."));
		return;
	}

	UGDMPropertyJsonSystemComponent* JsonSystemComponent = GetPropertyJsonSystemComponent();
	if (!IsValid(JsonSystemComponent))
	{
		UE_LOG(LogGDM, Error, TEXT("LoadDebugMenuFile: JsonSystemComponent not found on the same actor."));
		return;
	}

	FString LoadedJsonString;
	if (!LoadFile(LoadedJsonString))
	{
		return;
	}
	
	if (!JsonSystemComponent->BuildJsonFromString(LoadedJsonString))
	{
		UE_LOG(LogGDM, Warning, TEXT("LoadDebugMenuFile: Failed to apply JSON to JsonSystemComponent."));
		
		/* ファイルがないため現状の状態を１度保存する */
		SaveDebugMenuFile();
		return;
	}

	Manager->CallLoadedDebugMenuDispatcher();

	UE_LOG(LogGDM, Log, TEXT("LoadDebugMenuFile: JSON loaded and applied to JsonSystemComponent."));
}

void UGDMSaveSystemComponent::DeleteDebugMenuFile()
{
	AGameDebugMenuManager* Manager = Cast<AGameDebugMenuManager>(GetOwner());
	if (!IsValid(Manager))
	{
		UE_LOG(LogGDM, Error, TEXT("DeleteDebugMenuFile: GameDebugMenuManager not found on owner actor."));
		return;
	}

	if (!DeleteFile())
	{
		return;
	}

	Manager->CallDeletedDebugMenuDispatcher();

	UE_LOG(LogGDM, Log, TEXT("DeleteDebugMenuFile: JSON deleted and applied to JsonSystemComponent."));
}

UGDMPropertyJsonSystemComponent* UGDMSaveSystemComponent::GetPropertyJsonSystemComponent() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UGDMPropertyJsonSystemComponent>();
	}

	UE_LOG(LogGDM, Warning, TEXT("GetPropertyJsonSystemComponent: Owner is null or component not found."));
	
	return nullptr;
}

bool UGDMSaveSystemComponent::SaveFile(const FString& ContentString)
{
	if (CanUseSaveGame())
	{
		if (!IsValid(SaveGame))
		{
			SaveGame = Cast<UGDMSaveGame>(UGameplayStatics::CreateSaveGameObject(UGDMSaveGame::StaticClass()));
		}

		SaveGame->Json = ContentString;
		const FString SlotName = GetDefault<UGameDebugMenuSettings>()->SaveFileName;
		UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

		UE_LOG(LogGDM, Log, TEXT("SaveFile: JSON saved to SlotName '%s' UserIndex '%d'"), *SlotName, UserIndex);
		return true;
	}
	else
	{
		const FString SaveFilePath = GetDefault<UGameDebugMenuSettings>()->GetFullSavePath();

		if (FFileHelper::SaveStringToFile(ContentString, *SaveFilePath))
		{
			UE_LOG(LogGDM, Log, TEXT("SaveFile: JSON saved to '%s'"), *SaveFilePath);

			return true;
		}

		UE_LOG(LogGDM, Error, TEXT("SaveFile: Failed to save JSON to '%s'"), *SaveFilePath);
	}

	return false;
}

bool UGDMSaveSystemComponent::LoadFile(FString& OutLoadedContentString)
{
	OutLoadedContentString.Reset();
	
	if (CanUseSaveGame())
	{
		const FString SlotName = GetDefault<UGameDebugMenuSettings>()->SaveFileName;
		SaveGame = Cast<UGDMSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (!IsValid(SaveGame))
		{
			SaveGame = Cast<UGDMSaveGame>(UGameplayStatics::CreateSaveGameObject(UGDMSaveGame::StaticClass()));
		}
		else
		{
			UE_LOG(LogGDM, Log, TEXT("LoadFile: JSON loaded to SlotName '%s' UserIndex '%d'"), *SlotName, UserIndex);
		}

		OutLoadedContentString = SaveGame->Json;
		return true;
	}
	else
	{
		const FString LoadFilePath = GetDefault<UGameDebugMenuSettings>()->GetFullSavePath();
		
		if (FFileHelper::LoadFileToString(OutLoadedContentString, *LoadFilePath))
		{
			UE_LOG(LogGDM, Log, TEXT("LoadFile: JSON loaded to '%s'"), *LoadFilePath);
			return true;
		}

		UE_LOG(LogGDM, Warning, TEXT("LoadFile: Failed to load JSON to '%s'"), *LoadFilePath);
	}

	return false;
}

bool UGDMSaveSystemComponent::DeleteFile()
{
	if (CanUseSaveGame())
	{
		const FString SlotName = GetDefault<UGameDebugMenuSettings>()->SaveFileName;
		if (UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex))
		{
			return true;
		}
		
		UE_LOG(LogGDM, Warning, TEXT("DeleteFile: Failed to DeleteGameInSlot to '%s'"), *SlotName);
	}
	else
	{
		const FString FilePath = GetDefault<UGameDebugMenuSettings>()->GetFullSavePath();
		if (FFileHelper::SaveStringToFile(FString(), *FilePath))/* ファイル削除がないので空で上書き */
		{
			UE_LOG(LogGDM, Log, TEXT("DeleteFile: JSON saved to '%s'"), *FilePath);

			return true;
		}

		UE_LOG(LogGDM, Error, TEXT("DeleteFile: Failed to save JSON to '%s'"), *FilePath);
	}

	return false;
}

bool UGDMSaveSystemComponent::CanUseSaveGame()
{
	if (!GetDefault<UGameDebugMenuSettings>()->bUseSaveGame)
	{
		/* テキストファイルで直接書き込み */
		
#if (PLATFORM_WINDOWS || PLATFORM_MAC)
		/* 他のプラットフォームだとテキストが扱えないかもしれないため PC のみ */
		return false;
#endif
	}

	/* データはSaveGameを使用 */
	return true;
}
