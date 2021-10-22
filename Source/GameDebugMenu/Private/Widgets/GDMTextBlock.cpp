/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMTextBlock.h"
#include "GameDebugMenuSettings.h"

#define LOCTEXT_NAMESPACE "UMG"

UGDMTextBlock::UGDMTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGDMTextBlock::SynchronizeProperties()
{
	if (IsValid(UGameDebugMenuSettings::Get()->GetGDMFont()) != false)
	{
		Font.FontObject = UGameDebugMenuSettings::Get()->GetGDMFont();
	}
	Super::SynchronizeProperties();
}

#if WITH_EDITOR

const FText UGDMTextBlock::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif

#undef LOCTEXT_NAMESPACE
