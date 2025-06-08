/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "ConsoleCommand/GDMConsoleVariableCommandValueProvider.h"

bool UGDMConsoleVariableCommandValueProvider::GetFloatValue_Implementation(const FString& CommandName, float& OutValue) const
{
	if (const IConsoleVariable* Var = IConsoleManager::Get().FindConsoleVariable(*CommandName))
	{
		OutValue = Var->GetFloat();
		return true;
	}
	
	return false;
}
