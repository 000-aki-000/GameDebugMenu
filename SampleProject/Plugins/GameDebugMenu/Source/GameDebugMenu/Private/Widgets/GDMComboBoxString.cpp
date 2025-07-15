/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMComboBoxString.h"
#include <GameDebugMenuSettings.h>

#define LOCTEXT_NAMESPACE "GDMComboBoxString"

UGDMComboBoxString::UGDMComboBoxString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitIsFocusable(false);

	if(!IsRunningDedicatedServer())
	{
		InitFont(FSlateFontInfo(GetDefault<UGameDebugMenuSettings>()->GetDebugMenuFont(), 14, FName("Bold")));
	}
}

#if WITH_EDITOR

const FText UGDMComboBoxString::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif

#undef LOCTEXT_NAMESPACE