/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMTextBlock.h"
#include <Kismet/KismetTextLibrary.h>
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuManager.h"

#define LOCTEXT_NAMESPACE "UMG"

UGDMTextBlock::UGDMTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuStringKey()
#if WITH_EDITORONLY_DATA
	, PreviewLanguageKey(TEXT("Japanese"))
#endif
{
}

void UGDMTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if( IsValid(UGameDebugMenuSettings::Get()->GetGDMFont()) )
	{
		Font.FontObject = UGameDebugMenuSettings::Get()->GetGDMFont();
	}

	if( !DebugMenuStringKey.IsEmpty() )
	{
		SetText(FText::FromString(DebugMenuStringKey));
	}
}

void UGDMTextBlock::SetText(FText InText)
{
	FString StringKey = InText.ToString();

	if( DebugMenuStringKey != StringKey )
	{
		DebugMenuStringKey.Reset();
	}

	FString DebugMenuStr;

#if WITH_EDITORONLY_DATA
	if( !InText.IsEmpty() )
	{
		if( UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) )
		{
			/* テキストのStringKeyなら取得しセットするテキストを上書きする */

			if( World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview )
			{
				DebugMenuStr = UGameDebugMenuSettings::Get()->GetDebugMenuString(PreviewLanguageKey, StringKey);
				if( !DebugMenuStr.IsEmpty() )
				{
					DebugMenuStringKey = InText.ToString();
					InText = FText::FromString(DebugMenuStr);
				}
			}
			else if( AGameDebugMenuManager* Manager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this) )
			{
				if( Manager->GetDebugMenuString(StringKey, DebugMenuStr) )
				{
					DebugMenuStringKey = InText.ToString();
					InText = FText::FromString(DebugMenuStr);
				}
			}
		}
	}
#else
	if( !InText.IsEmpty() )
	{
		if( UGameDebugMenuFunctions::GetDebugMenuString(this, StringKey, DebugMenuStr) )
		{
			DebugMenuStringKey = InText.ToString();
			InText = FText::FromString(DebugMenuStr);
		}
	}
#endif

	Super::SetText(InText);
}

#if WITH_EDITOR

const FText UGDMTextBlock::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif

#undef LOCTEXT_NAMESPACE
