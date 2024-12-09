// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GDMSaveSystemComponent.h"

#include "GameDebugMenuManager.h"
#include "GameDebugMenuSettings.h"
#include "Component/GDMPropertyJsonSystemComponent.h"

UGDMSaveSystemComponent::UGDMSaveSystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGDMSaveSystemComponent::SaveDebugMenuFile() const
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
	if (!JsonSystemComponent)
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

	const FString SaveFilePath = UGameDebugMenuSettings::Get()->GetFullSavePath();
	if (FFileHelper::SaveStringToFile(JsonString, *SaveFilePath))
	{
		UE_LOG(LogGDM, Log, TEXT("SaveDebugMenuFile: JSON saved to '%s'"), *SaveFilePath);
		return;
	}

	UE_LOG(LogGDM, Error, TEXT("SaveDebugMenuFile: Failed to save JSON to '%s'"), *SaveFilePath);
}

void UGDMSaveSystemComponent::LoadDebugMenuFile() const
{
	if (UGameDebugMenuSettings::Get()->bDisableSaveFile)
	{
		return;
	}

	const FString LoadFilePath = UGameDebugMenuSettings::Get()->GetFullSavePath();

	FString LoadedJsonString;
	if (!FFileHelper::LoadFileToString(LoadedJsonString, *LoadFilePath))
	{
		UE_LOG(LogGDM, Log, TEXT("LoadDebugMenuFile: Failed to load JSON from '%s'"), *LoadFilePath);
		return;
	}
	
	UGDMPropertyJsonSystemComponent* JsonSystemComponent = GetPropertyJsonSystemComponent();
	if (!JsonSystemComponent)
	{
		UE_LOG(LogGDM, Error, TEXT("LoadDebugMenuFile: JsonSystemComponent not found on the same actor."));
		return;
	}
	
	if (!JsonSystemComponent->BuildJsonFromString(LoadedJsonString))
	{
		UE_LOG(LogGDM, Error, TEXT("LoadDebugMenuFile: Failed to apply JSON to JsonSystemComponent."));
		return;
	}

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