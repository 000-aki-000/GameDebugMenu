/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "GDMComboBoxString.generated.h"

/**
 * 
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMComboBoxString : public UComboBoxString
{
	GENERATED_BODY()
	
public:
	UGDMComboBoxString(const FObjectInitializer& ObjectInitializer);
public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

};
