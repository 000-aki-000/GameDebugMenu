/**
* Copyright (c) 2022 akihiko moroi
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
	bIsFocusable = false;

	if(!IsRunningDedicatedServer())
	{
		Font = FSlateFontInfo(UGameDebugMenuSettings::Get()->GetGDMFont(), 14, FName("Bold"));
	}
}

#if WITH_EDITOR

const FText UGDMComboBoxString::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif