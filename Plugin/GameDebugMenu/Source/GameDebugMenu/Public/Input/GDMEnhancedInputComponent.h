/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GDMEnhancedInputComponent.generated.h"


/**
 * DebugMenu用のUIで入力判定を制御する専用のEnhancedInputComponent
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UGDMEnhancedInputComponent();

	virtual bool CanProcessInputAction(const UInputAction* Action) const;
	FEnhancedInputActionEventBinding& BindDebugMenuAction(const UInputAction* Action, ETriggerEvent TriggerEvent, UObject* Object, FName FunctionName);
	FEnhancedInputActionEventBinding& BindDebugMenuAction(const UInputAction* Action, ETriggerEvent TriggerEvent, TFunction<void(const FInputActionInstance&)>&& Callback);
};
