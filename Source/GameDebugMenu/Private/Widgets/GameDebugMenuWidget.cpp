/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuWidget.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuFunctions.h"
#include <Kismet/GameplayStatics.h>
#include <GDMPlayerControllerProxyComponent.h>

void UGameDebugMenuWidget::SendSelfEvent(FName EventName)
{
	OnSendWidgetEventDispatcher.Broadcast(this, EventName);
}

void UGameDebugMenuWidget::ExecuteGDMConsoleCommand(const FString Command, EGDMConsoleCommandNetType CommandNetType)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
	{
		return;
	}

	UGDMPlayerControllerProxyComponent* DebugMenuPCProxyComponent = PC->FindComponentByClass<UGDMPlayerControllerProxyComponent>();
	if (IsValid(DebugMenuPCProxyComponent) == false)
	{
		return;
	}

	DebugMenuPCProxyComponent->ExecuteConsoleCommand(Command, CommandNetType);
}

bool UGameDebugMenuWidget::IsActivateDebugMenu()
{
	return bActivateMenu;
}

void UGameDebugMenuWidget::ActivateDebugMenu(bool bAlwaysExecute)
{
	if (bAlwaysExecute == false)
	{
		if (bActivateMenu != false)
		{
			return;
		}
	}

	bActivateMenu = true;
	OnActivateDebugMenu(bAlwaysExecute);
}

void UGameDebugMenuWidget::DeactivateDebugMenu()
{
	if (bActivateMenu == false)
	{
		return;
	}

	bActivateMenu = false;
	OnDeactivateDebugMenu();
}
