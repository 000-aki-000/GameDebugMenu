/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuWidget.h"
#include "GameDebugMenuRootWidget.generated.h"

class AGameDebugMenuManager;

/**
 * DebugMenuのルートに当たるWidget
 */
UCLASS()
class GAMEDEBUGMENU_API UGameDebugMenuRootWidget : public UGameDebugMenuWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	AGameDebugMenuManager* DebugMenuManager;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|Event")
	void ShowDebugReport();
};
