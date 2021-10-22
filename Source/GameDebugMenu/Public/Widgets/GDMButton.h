/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "GDMButton.generated.h"

/**
 * GameDebugMenu用のボタン
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMButton : public UButton
{
	GENERATED_BODY()
	
		
public:
	UGDMButton(const FObjectInitializer& ObjectInitializer);
public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:
	FReply GDMSlateHandleClicked();
	void GDMSlateHandlePressed();
	void GDMSlateHandleReleased();
	void GDMSlateHandleHovered();
	void GDMSlateHandleUnhovered();
};
