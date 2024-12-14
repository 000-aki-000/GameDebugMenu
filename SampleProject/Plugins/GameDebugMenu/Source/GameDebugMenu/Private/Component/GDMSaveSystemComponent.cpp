// Fill out your copyright notice in the Description page of Project Settings.


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
	if (UGameDebugMenuSettings::Get()->bDisableSaveFile)
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
	Manager->GetOutputCommandHistoryString(CommandHistory);
	JsonSystemComponent->SetStringArrayToJson(TEXT("CommandHistory"), CommandHistory);
	
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
	if (UGameDebugMenuSettings::Get()->bDisableSaveFile)
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
		UE_LOG(LogGDM, Error, TEXT("LoadDebugMenuFile: Failed to apply JSON to JsonSystemComponent."));
		
		return;
	}

	Manager->CallLoadedDebugMenuDispatcher();

	UE_LOG(LogGDM, Log, TEXT("LoadDebugMenuFile: JSON loaded and applied to JsonSystemComponent."));
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
		const FString SlotName = UGameDebugMenuSettings::Get()->SaveFileName;
		UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);

		UE_LOG(LogGDM, Log, TEXT("SaveFile: JSON saved to SlotName '%s' UserIndex '%d'"), *SlotName, UserIndex);
		return true;
	}
	else
	{
		const FString SaveFilePath = UGameDebugMenuSettings::Get()->GetFullSavePath();

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
		const FString SlotName = UGameDebugMenuSettings::Get()->SaveFileName;
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
		const FString LoadFilePath = UGameDebugMenuSettings::Get()->GetFullSavePath();
		
		if (FFileHelper::LoadFileToString(OutLoadedContentString, *LoadFilePath))
		{
			UE_LOG(LogGDM, Log, TEXT("LoadFile: JSON loaded to '%s'"), *LoadFilePath);
			return true;
		}

		UE_LOG(LogGDM, Warning, TEXT("LoadFile: Failed to load JSON to '%s'"), *LoadFilePath);
	}

	return false;
}

bool UGDMSaveSystemComponent::CanUseSaveGame()
{
	if (!UGameDebugMenuSettings::Get()->bUseSaveGame)
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
