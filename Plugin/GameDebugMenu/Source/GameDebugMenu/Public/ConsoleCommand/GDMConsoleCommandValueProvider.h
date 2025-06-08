/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GDMConsoleCommandValueProvider.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GAMEDEBUGMENU_API UGDMConsoleCommandValueProvider : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetFloatValue(const FString& CommandName, float& OutValue) const;
	virtual bool GetFloatValue_Implementation(const FString& CommandName, float& OutValue) const { return false; }

};
