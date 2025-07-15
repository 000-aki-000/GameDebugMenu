/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMLocalizeStringComponent.h"

#include "GameDebugMenuSettings.h"
#include "Component/GDMPropertyJsonSystemComponent.h"
#include "Internationalization/StringTableCore.h"

UGDMLocalizeStringComponent::UGDMLocalizeStringComponent()
	: CachedDebugMenuStrings()
	, bCurrentDebugMenuDirectStringKey(false)
	, CurrentLanguage(NAME_None)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGDMLocalizeStringComponent::SetJsonSystemComponentValue(UGDMPropertyJsonSystemComponent* PropertyJsonSystemComponent)
{
	FString StringKey = TEXT("DebugMenuDirectStringKey");
	{
		if (!PropertyJsonSystemComponent->HasCustomString(StringKey))
		{
			PropertyJsonSystemComponent->SetCustomString(StringKey, TEXT("False"));
		}

		bCurrentDebugMenuDirectStringKey = PropertyJsonSystemComponent->GetCustomString(StringKey, TEXT("False")).ToBool();
	}
	
	StringKey = TEXT("DebugMenuLanguage");
	{
		if (!PropertyJsonSystemComponent->HasCustomString(StringKey))
		{
			PropertyJsonSystemComponent->SetCustomString(StringKey, GetDefault<UGameDebugMenuSettings>()->DefaultGameDebugMenuLanguage.ToString());
		}

		CurrentLanguage = *PropertyJsonSystemComponent->GetCustomString(StringKey, GetDefault<UGameDebugMenuSettings>()->DefaultGameDebugMenuLanguage.ToString());
	}
}

void UGDMLocalizeStringComponent::SetToJsonSystemComponent(UGDMPropertyJsonSystemComponent* PropertyJsonSystemComponent, const FString& Language)
{
	CurrentLanguage = *Language;
	PropertyJsonSystemComponent->SetCustomString(TEXT("DebugMenuDirectStringKey"), bCurrentDebugMenuDirectStringKey ? TEXT("True") : TEXT("False"));
	PropertyJsonSystemComponent->SetCustomString(TEXT("DebugMenuLanguage"), Language);
}

void UGDMLocalizeStringComponent::SyncLoadDebugMenuStringTables()
{
	CachedDebugMenuStrings.Reset();

	if( const FGDMStringTableList* StringTableList = GetDefault<UGameDebugMenuSettings>()->TryGetStringTableList(CurrentLanguage) )
	{
		UE_LOG(LogGDM, Verbose, TEXT("Call SyncLoadDebugMenuStringTables %s"), *CurrentLanguage.ToString());
		
		for( auto& StrTablePtr : StringTableList->StringTables )
		{
			if( !StrTablePtr.ToSoftObjectPath().IsValid() || StrTablePtr.ToSoftObjectPath().IsNull() )
			{
				UE_LOG(LogGDM, Warning, TEXT("SyncLoadDebugMenuStringTables: failed StringTable : LanguageKey->%s"), *CurrentLanguage.ToString());
				continue;
			}
				
			if(const UStringTable* StringTable = StrTablePtr.LoadSynchronous() )
			{
				StringTable->GetStringTable()->EnumerateSourceStrings([&](const FString& InKey, const FString& InSourceString) -> bool
				{
					if( !CachedDebugMenuStrings.Contains(InKey) )
					{
						CachedDebugMenuStrings.Add(InKey, InSourceString);
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
		UE_LOG(LogGDM, Error, TEXT("Failed Load DebugMenuStringTables %s"), *CurrentLanguage.ToString());
	}
}