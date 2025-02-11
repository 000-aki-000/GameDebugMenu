/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Data/GameDebugMenuManagerAsset.h"

#include "Internationalization/StringTableCore.h"

UGameDebugMenuManagerAsset::UGameDebugMenuManagerAsset()
	: Super()
	, DebugMenuManagerClass(nullptr)
{
	GameDebugMenuStringTables.Reset();
	
	const FName Japanese = TEXT("Japanese");
	{
		FGDMStringTableList StringTableList;
		StringTableList.StringTables.Add(TSoftObjectPtr<UStringTable>(FSoftObjectPath(TEXT("/GameDebugMenu/StringTable/ST_GDM_Japanese.ST_GDM_Japanese"))));
		GameDebugMenuStringTables.Add(Japanese, StringTableList);
	}

	const FName English = TEXT("English");
	{
		FGDMStringTableList StringTableList;
		StringTableList.StringTables.Add(TSoftObjectPtr<UStringTable>(FSoftObjectPath(TEXT("/GameDebugMenu/StringTable/ST_GDM_English.ST_GDM_English"))));
		GameDebugMenuStringTables.Add(English, StringTableList);
	}
}

FPrimaryAssetId UGameDebugMenuManagerAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("GameDebugMenu"), GetFName());
}

FString UGameDebugMenuManagerAsset::GetDebugMenuString(const FName& LanguageKey, const FString& StringKey) const
{
	if( const FGDMStringTableList* StringTableList = GameDebugMenuStringTables.Find(LanguageKey) )
	{
		FString OutSourceString;

		for( auto& StrTablePtr : StringTableList->StringTables )
		{
			if(const UStringTable* StringTable = StrTablePtr.LoadSynchronous() )
			{
				if( StringTable->GetStringTable()->GetSourceString(StringKey, OutSourceString) )
				{
					return OutSourceString;
				}
			}
		}		
	}

	return FString();
}