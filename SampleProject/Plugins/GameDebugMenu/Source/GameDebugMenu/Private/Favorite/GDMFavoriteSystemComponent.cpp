/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Favorite/GDMFavoriteSystemComponent.h"

#include "GameDebugMenuManager.h"
#include "GameDebugMenuTypes.h"
#include "Blueprint/UserWidget.h"
#include "Component/GDMPropertyJsonSystemComponent.h"
#include "Data/GameDebugMenuManagerAsset.h"
#include "Favorite/GDMFavoriteItemDefinition.h"

UGDMFavoriteSystemComponent::UGDMFavoriteSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

UGDMPropertyJsonSystemComponent* UGDMFavoriteSystemComponent::GetPropertyJsonSystemComponent() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UGDMPropertyJsonSystemComponent>();
	}

	UE_LOG(LogGDM, Warning, TEXT("GetPropertyJsonSystemComponent: Owner is null or component not found."));
	
	return nullptr;
}

void UGDMFavoriteSystemComponent::Initialize(const UGameDebugMenuManagerAsset* InMenuAsset)
{
	if (!IsValid(InMenuAsset))
	{
		UE_LOG(LogGDM, Warning, TEXT("UGDMFavoriteSystemComponent::Initialize: Invalid MenuDataAsset."));
		return;
	}

	CachedFavoriteDefinitions = InMenuAsset->FavoriteItemDefinitions;
	
	for (auto& Def : CachedFavoriteDefinitions)
	{
		Def->OwnerManager = Cast<AGameDebugMenuManager>(GetOwner());
	}
}

bool UGDMFavoriteSystemComponent::ToggleAddOrRemoveFavorite(UUserWidget* TargetWidget)
{
	if (CanFavorite(TargetWidget))
	{
		if (IsFavorited(TargetWidget))
		{
			RemoveFavorite(TargetWidget);
		}
		else
		{
			AddFavorite(TargetWidget);
		}
		return true;
	}

	return false;
}

void UGDMFavoriteSystemComponent::AddFavorite(UUserWidget* TargetWidget)
{
	if (!IsValid(TargetWidget))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddFavorite: Invalid target or MenuDataAsset."));
		return;
	}

	UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent();
	if (!IsValid(JsonSystem))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddFavorite: JsonSystem not found."));
		return;
	}

	for (const auto& Def : CachedFavoriteDefinitions)
	{
		if (!IsValid(Def) || !Def->FavoriteWidgetClass)
		{
			continue;
		}

		if (!TargetWidget->IsA(Def->FavoriteWidgetClass))
		{
			continue;
		}

		const FString SaveKey = Def->ExportToFavoriteKey(TargetWidget);
		if (SaveKey.IsEmpty())
		{
			UE_LOG(LogGDM, Warning, TEXT("AddFavorite: ExportToFavoriteKey failed or returned empty."));
			return;
		}

		const FString DefinitionName = Def->GetClass()->GetName();

		if (JsonSystem->HasFavoriteEntry(DefinitionName, SaveKey))
		{
			UE_LOG(LogGDM, Log, TEXT("AddFavorite: Already exists (%s, %s)"), *DefinitionName, *SaveKey);
			return;
		}

		JsonSystem->AddFavoriteEntry(DefinitionName, SaveKey);

		UE_LOG(LogGDM, Verbose, TEXT("AddFavorite: Added (%s, %s)"), *DefinitionName, *SaveKey);
		return;
	}

	UE_LOG(LogGDM, Warning, TEXT("AddFavorite: No matching FavoriteItemDefinition found for widget class %s"), *GetNameSafe(TargetWidget->GetClass()));
}

void UGDMFavoriteSystemComponent::RemoveFavorite(UUserWidget* TargetWidget)
{
	if (!IsValid(TargetWidget))
	{
		return;
	}

	UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent();
	if (!IsValid(JsonSystem))
	{
		return;
	}

	for (const auto& Def : CachedFavoriteDefinitions)
	{
		if (!IsValid(Def) || !TargetWidget->IsA(Def->FavoriteWidgetClass))
		{
			continue;
		}

		const FString SaveKey = Def->ExportToFavoriteKey(TargetWidget);
		if (SaveKey.IsEmpty())
		{
			continue;
		}

		const FString DefinitionName = Def->GetClass()->GetName();
		JsonSystem->RemoveFavoriteEntry(DefinitionName, SaveKey);
		return;
	}
}

bool UGDMFavoriteSystemComponent::CanFavorite(UUserWidget* TargetWidget)
{
	if (!IsValid(TargetWidget))
	{
		UE_LOG(LogGDM, Warning, TEXT("AddFavorite: TargetWidget is invalid."));
		return false;
	}

	for (const auto& Def : CachedFavoriteDefinitions)
	{
		if (!IsValid(Def) || !Def->FavoriteWidgetClass)
		{
			continue;
		}

		if (!TargetWidget->IsA(Def->FavoriteWidgetClass))
		{
			continue;
		}
		
		if (!Def->IsSupportedWidget(TargetWidget))
		{
			continue;
		}
		
		/* お気に入り指定可能 */
		return true;
	}

	return false;
}

bool UGDMFavoriteSystemComponent::IsFavorited(UUserWidget* TargetWidget)
{
	if (!IsValid(TargetWidget))
	{
		return false;
	}

	UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent();
	if (!IsValid(JsonSystem))
	{
		return false;
	}

	for (const auto& Def : CachedFavoriteDefinitions)
	{
		if (!IsValid(Def) || !TargetWidget->IsA(Def->FavoriteWidgetClass))
		{
			continue;
		}

		const FString SaveKey = Def->ExportToFavoriteKey(TargetWidget);
		if (SaveKey.IsEmpty())
		{
			continue;
		}

		const FString DefinitionName = Def->GetClass()->GetName();
		if (JsonSystem->HasFavoriteEntry(DefinitionName, SaveKey))
		{
			return true;
		}
	}
	
	return false;
}

void UGDMFavoriteSystemComponent::ClearAllFavorites()
{
	if (UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent())
	{
		const TArray<FGDMFavoriteEntry> All = JsonSystem->GetAllFavoriteEntries();
		
		for (const FGDMFavoriteEntry& Entry : All)
		{
			JsonSystem->RemoveFavoriteEntry(Entry.DefinitionName, Entry.SaveKey);
		}
	}
}

bool UGDMFavoriteSystemComponent::IsFavoriteListEqualTo(const TArray<FGDMFavoriteEntry>& OtherList) const
{
	if (const UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent())
	{
		const TArray<FGDMFavoriteEntry> Current = JsonSystem->GetAllFavoriteEntries();

		if (Current.Num() != OtherList.Num())
		{
			/* 件数が異なるのでNG */
			return false;
		}

		for (int32 i = 0; i < Current.Num(); ++i)
		{
			if (Current[i].DefinitionName != OtherList[i].DefinitionName ||
				Current[i].SaveKey != OtherList[i].SaveKey)
			{
				/* いずれかのデータが合わないのでNG */
				return false;
			}
		}

		return true;
	}

	/* JSONが取得できなければ常に差異ありとみなす */
	return false;
}

UUserWidget* UGDMFavoriteSystemComponent::CreateFavoriteWidgetFromEntry(const FGDMFavoriteEntry& Entry)
{
	if (UGDMPropertyJsonSystemComponent* JsonSystem = GetPropertyJsonSystemComponent())
	{
		for (UGDMFavoriteItemDefinition* Def : CachedFavoriteDefinitions)
		{
			if (IsValid(Def) && Def->GetClass()->GetName() == Entry.DefinitionName)
			{
				return Def->CreateWidgetFromFavoriteData(Entry.SaveKey);
			}
		}
	}
	
	UE_LOG(LogGDM, Warning, TEXT("CreateFavoriteWidgetFromEntry: Definition not found: %s"), *Entry.DefinitionName);
	return nullptr;
}
