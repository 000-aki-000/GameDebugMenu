/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMComboBoxString.h"
#include <GameDebugMenuSettings.h>

#define LOCTEXT_NAMESPACE "UMG"

UGDMComboBoxString::UGDMComboBoxString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitIsFocusable(false);

	if(!IsRunningDedicatedServer())
	{
		InitFont(FSlateFontInfo(UGameDebugMenuSettings::Get()->GetGDMFont(), 14, FName("Bold")));
	}
}

#if WITH_EDITOR

const FText UGDMComboBoxString::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif