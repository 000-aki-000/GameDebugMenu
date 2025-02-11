/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMFunctionWidget.h"
#include "GameDebugMenuFunctions.h"

bool UGDMFunctionWidget::TryCallObjectFunction(FName EventName)
{
	return GDMProcessEvent(EventName, nullptr);
}

bool UGDMFunctionWidget::GDMProcessEvent(FName EventName, void* Parms)
{
	if(!IsValid(TargetObject))
	{
		UE_LOG(LogGDM, Warning, TEXT("GDMFunctionWidget ProcessEvent: Not found Object"));
		return false;
	}

	if(EventName == NAME_None)
	{
		UE_LOG(LogGDM, Warning, TEXT("GDMFunctionWidget ProcessEvent: Not found EventName"));
		return false;
	}

	UFunction* TargetFunction = TargetObject->FindFunction(EventName);
	if(!IsValid(TargetFunction))
	{
		UE_LOG(LogGDM, Warning, TEXT("GDMFunctionWidget ProcessEvent: Not found Function"));
		return false;
	}

	if(TargetFunction->NumParms != 0)
	{
		UE_LOG(LogGDM, Warning, TEXT("GDMFunctionWidget ProcessEvent: Arguments are not supported: %d"), TargetFunction->NumParms);
		return false;
	}

	TargetObject->ProcessEvent(TargetFunction, nullptr);

	UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->CallExecuteProcessEventDispatcher(EventName, TargetObject);
	return true;
}
