/**
* Copyright (c) 2022 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMPadInputWidgetController.h"
#include <Widgets/GameDebugMenuWidget.h>
#include <GDMListenerComponent.h>

UWorld* UGDMPadInputWidgetController::GetWorld() const
{
	if( !IsValid(OwnerGameDebugMenuWidget) )
	{
		return GWorld;
	}
	return OwnerGameDebugMenuWidget->GetWorld();
}

void UGDMPadInputWidgetController::InitializeListenerComponent(UObject* WorldContextObject)
{
	if( !IsValid(ListenerComponent) )
	{
		if( UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) )
		{
			ListenerComponent = NewObject<UGDMListenerComponent>(World, NAME_None, RF_Transient);
			UGDMListenerComponent::PushListenerComponent(World,ListenerComponent);

			FWorldDelegates::OnPreWorldFinishDestroy.AddUObject(this, &UGDMPadInputWidgetController::OnPreWorldFinishDestroy);
		}
	}
}

UGDMListenerComponent* UGDMPadInputWidgetController::GetListenerComponent()
{
	return ListenerComponent;
}

void UGDMPadInputWidgetController::OnPreWorldFinishDestroy(UWorld* World)
{
	if( IsValid(ListenerComponent) && ListenerComponent->GetWorld() == World )
	{
		ListenerComponent->AllUnbindDispatchers();
		UGDMListenerComponent::PopListenerComponent(World, ListenerComponent);
		ListenerComponent = nullptr;
	}
}