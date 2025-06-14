/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GDMConsoleCommandValueProvider.h"
#include "GDMConsoleVariableCommandValueProvider.generated.h"

/**
 * ConsoleManager用プロバイダー
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMConsoleVariableCommandValueProvider : public UGDMConsoleCommandValueProvider
{
	GENERATED_BODY()

public:
	virtual bool GetFloatValue_Implementation(const FString& CommandName, float& OutValue) const override;
};
