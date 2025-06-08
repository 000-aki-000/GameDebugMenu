/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "ConsoleCommand/GDMSlomoCommandValueProvider.h"

bool UGDMSlomoCommandValueProvider::GetFloatValue_Implementation(const FString& CommandName, float& OutValue) const
{
	if (const UWorld* World = GetWorld())
	{
		if (IsValid(World->GetWorldSettings()))
		{
			OutValue = World->GetWorldSettings()->GetEffectiveTimeDilation();
			return true;
		}
	}
	
	return false;
}
