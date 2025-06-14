/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "GDMPadInputWidgetController.generated.h"

class UGDMListenerComponent;
class UGameDebugMenuWidget;

/**
 * 
 */
UCLASS(Blueprintable)
class GAMEDEBUGMENU_API UGDMPadInputWidgetController : public UObject
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn))
	UGameDebugMenuWidget* OwnerGameDebugMenuWidget;

public:
	virtual UWorld* GetWorld() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 NextChoosing();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 PreviousChoosing();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	UWidget* GetChoosingWidget() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 GetChoosingWidgetIndex() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	UWidget* GetChosenWidget() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 GetChosenWidgetIndex() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	bool IsChosen() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void ChangeChosenMode();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void ChangeChoosingMode();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void SetChoosingWidget(UWidget* NewChoosingWidget);
};
