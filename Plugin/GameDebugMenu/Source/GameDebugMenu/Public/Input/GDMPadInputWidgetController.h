/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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
	UPROPERTY(Transient, DuplicateTransient)
	UGDMListenerComponent* ListenerComponent;

	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn))
	UGameDebugMenuWidget* OwnerGameDebugMenuWidget;

public:
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "GDM|ListenerComponent", meta = (WorldContext = "WorldContextObject"))
	void InitializeListenerComponent(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "GDM|ListenerComponent")
	UGDMListenerComponent* GetListenerComponent();


private:
	void OnPreWorldFinishDestroy(UWorld* World);

};
