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
	UPROPERTY(BlueprintReadOnly,EditAnywhere, Category = "GDM|Config")
	FString DebugMenuStringKey;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "GDM|Config|Editor")
	FName PreviewLanguageKey;
#endif

public:
	UGDMTextBlock(const FObjectInitializer& ObjectInitializer);

public:
	virtual void SynchronizeProperties() override;
	virtual void SetText(FText InText) override;

	UFUNCTION(BlueprintCallable, Category = "Appearance")
	virtual void SetWrapTextAt(float InWrapTextAt);

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
