/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "ConsoleCommand/GDMConsoleCommandValueProviderComponent.h"

#include "ConsoleCommand/GDMConsoleVariableCommandValueProvider.h"


UGDMConsoleCommandValueProviderComponent::UGDMConsoleCommandValueProviderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UGDMConsoleCommandValueProviderComponent::GetFloatValue(const FString& CommandName, float& OutValue)
{
	for (const FGDMConsoleCommandProviderPattern& PatternStruct : ProviderPatterns)
	{
		if (PatternStruct.Pattern.IsEmpty() || !IsValid(PatternStruct.Provider))
		{
			continue;
		}
		
		if (CommandName.Contains(PatternStruct.Pattern))
		{
			return PatternStruct.Provider->GetFloatValue(CommandName, OutValue);
		}
	}
	
	if (IsValid(ConsoleVariableCommandValueProvider))
	{
		return ConsoleVariableCommandValueProvider->GetFloatValue(CommandName, OutValue);
	}
	
	return false;
}


