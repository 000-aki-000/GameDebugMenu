/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "GDMTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMTextBlock : public UTextBlock
{
	GENERATED_BODY()
	

public:
	UGDMTextBlock(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SynchronizeProperties() override;

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
};
