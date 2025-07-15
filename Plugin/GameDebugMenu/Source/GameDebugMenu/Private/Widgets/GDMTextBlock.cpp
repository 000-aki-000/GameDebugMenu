/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMTextBlock.h"
#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuManager.h"
#include "Component/GDMLocalizeStringComponent.h"
#include "Engine/Engine.h"
#include "Widgets/GameDebugMenuRootWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"

UGDMTextBlock::UGDMTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuStringKey()
#if WITH_EDITORONLY_DATA
	, PreviewLanguageKey(TEXT("Japanese"))
#endif
{
	if( !IsRunningDedicatedServer() )
	{
		SetFont(FSlateFontInfo(GetDefault<UGameDebugMenuSettings>()->GetDebugMenuFont(), 24, FName("Bold")));
	}
}

void UGDMTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

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

	UWorld* World = GetWorld();
	if( !IsValid(World) )
	{
		if( UGameDebugMenuRootWidget* RootWidget = Cast<UGameDebugMenuRootWidget>(GetOuter()) )
		{
			World = RootWidget->GetOwnerManager()->GetWorld();
		}
		else
		{
			World = GEngine->GetWorldFromContextObject(World, EGetWorldErrorMode::LogAndReturnNull);
		}
	}

	FString DebugMenuStr;

#if WITH_EDITORONLY_DATA
	if( !InText.IsEmpty() )
	{
		if( IsValid(World) )
		{
			/* テキストのStringKeyなら取得しセットするテキストを上書きする */

			if( World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview )
			{
				DebugMenuStr = GetDefault<UGameDebugMenuSettings>()->GetDebugMenuString(PreviewLanguageKey, StringKey);
				if( !DebugMenuStr.IsEmpty() )
				{
					DebugMenuStringKey = InText.ToString();
					InText = FText::FromString(DebugMenuStr);
				}
			}
			else if( AGameDebugMenuManager* Manager = UGameDebugMenuFunctions::GetGameDebugMenuManager(World) )
			{
				if( Manager->GetLocalizeStringComponent()->GetString(StringKey, DebugMenuStr) )
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
		if( UGameDebugMenuFunctions::GetDebugMenuString(World, StringKey, DebugMenuStr) )
		{
			DebugMenuStringKey = InText.ToString();
			InText = FText::FromString(DebugMenuStr);
		}
	}
#endif

	Super::SetText(InText);
}

void UGDMTextBlock::SetWrapTextAt(float InWrapTextAt)
{
	WrapTextAt = InWrapTextAt;
	if( MyTextBlock.IsValid() )
	{
		MyTextBlock->SetWrapTextAt(InWrapTextAt);
	}
}

#if WITH_EDITOR

const FText UGDMTextBlock::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

bool UGDMTextBlock::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}

#endif

#undef LOCTEXT_NAMESPACE
