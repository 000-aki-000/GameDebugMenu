/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMEnhancedInputComponent.h"

#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuSettings.h"

UGDMEnhancedInputComponent::UGDMEnhancedInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	/* 最後に追加されたUI用InputComponentだけ処理したい（アンバインド処理をしなくてもいいように）のでブロック指定にする */
	bBlockInput = true;

	/* DebugMenuはEnhancedInputを利用するので他のInputComponentより優先度は高くなるような値を指定 */
	Priority = GetDefault<UGameDebugMenuSettings>()->WidgetInputActionPriority;
}

bool UGDMEnhancedInputComponent::CanProcessInputAction(const UInputAction* Action) const
{
	AGameDebugMenuManager* Manager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this);
	if (!IsValid(Manager))
	{
		return false;
	}

	if (Manager->IsInputIgnored())
	{
		return false;
	}
	
	
	return true;
}

FEnhancedInputActionEventBinding& UGDMEnhancedInputComponent::BindDebugMenuAction(const UInputAction* Action, ETriggerEvent TriggerEvent, UObject* Object, FName FunctionName)
{
	return BindActionInstanceLambda(
		Action,
		TriggerEvent,
		[this, Object, FunctionName, Action](const FInputActionInstance& Instance)
		{
			if (CanProcessInputAction(Action) && IsValid(Object))
			{
				if (UFunction* Func = Object->FindFunction(FunctionName))
				{
					Object->ProcessEvent(Func, nullptr);
				}
			}
		});
}

FEnhancedInputActionEventBinding& UGDMEnhancedInputComponent::BindDebugMenuAction(const UInputAction* Action, ETriggerEvent TriggerEvent, TFunction<void(const FInputActionInstance&)>&& InCallback)
{
	return BindActionInstanceLambda(
		Action,
		TriggerEvent,
		[this, Action, Callback = MoveTemp(InCallback)](const FInputActionInstance& Instance)
		{
			if (CanProcessInputAction(Action))
			{
				Callback(Instance);
			}
		});
}
