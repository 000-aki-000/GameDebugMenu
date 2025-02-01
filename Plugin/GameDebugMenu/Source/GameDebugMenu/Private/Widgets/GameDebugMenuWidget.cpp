/**
* Copyright (c) 2025 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuWidget.h"
#include <Blueprint/WidgetTree.h>
#include "TimerManager.h"
#include "GameDebugMenuManager.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Engine/DebugCameraController.h"
#include "Engine/LocalPlayer.h"

void UGameDebugMenuWidget::SendSelfEvent(FName EventName)
{
	OnSendWidgetEventDispatcher.Broadcast(this, EventName);
}

void UGameDebugMenuWidget::ExecuteGDMConsoleCommand(const FString Command, const EGDMConsoleCommandNetType CommandNetType)
{
	if (const APlayerController* PC = GetOriginalPlayerController() )
	{
		if( UGDMPlayerControllerProxyComponent* DebugMenuPCProxyComponent = PC->FindComponentByClass<UGDMPlayerControllerProxyComponent>() )
		{
			DebugMenuPCProxyComponent->ExecuteConsoleCommand(Command, CommandNetType);
		}
	}
}

bool UGameDebugMenuWidget::IsActivateDebugMenu()
{
	return bActivateMenu;
}

void UGameDebugMenuWidget::ActivateDebugMenu(bool bAlwaysExecute)
{
	if (!bAlwaysExecute)
	{
		if (bActivateMenu)
		{
			return;
		}
	}

	bActivateMenu = true;
	OnActivateDebugMenu(bAlwaysExecute);
}

void UGameDebugMenuWidget::DeactivateDebugMenu()
{
	if (!bActivateMenu)
	{
		return;
	}

	bActivateMenu = false;
	OnDeactivateDebugMenu();
}

void UGameDebugMenuWidget::OnChangeDebugMenuLanguage(const FName& NewLanguageKey, const FName& OldLanguageKey)
{
	OnChangeDebugMenuLanguageBP(NewLanguageKey, OldLanguageKey);
}

bool UGameDebugMenuWidget::GetWidgetChildrenOfClass(TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutChildWidgets, bool bEndSearchAsYouFind)
{
	OutChildWidgets.Reset();

	/* 現在チェックするWidget郡 */
	TInlineComponentArray<UWidget*> WidgetsToCheck;

	/* チェック済みWidget郡 */
	TInlineComponentArray<UWidget*> CheckedWidgets;

	/* 作業用 */
	TArray<UWidget*> WorkWidgets;

	WidgetsToCheck.Push(this);

	while( WidgetsToCheck.Num() > 0 )
	{
		UWidget* PossibleParent = WidgetsToCheck.Pop(EAllowShrinking::No);

		if( CheckedWidgets.Contains(PossibleParent) )
		{
			/* チェック済み */
			continue;
		}

		CheckedWidgets.Add(PossibleParent);

		WorkWidgets.Reset();

		if(const UUserWidget* UserWidget = Cast<UUserWidget>(PossibleParent) )
		{
			if( UserWidget->WidgetTree != nullptr )
			{
				UserWidget->WidgetTree->GetAllWidgets(WorkWidgets);
			}
		}
		else
		{
			UWidgetTree::GetChildWidgets(PossibleParent, WorkWidgets);
		}

		for( UWidget* Widget : WorkWidgets )
		{
			if( CheckedWidgets.Contains(Widget) )
			{
				/* チェック済み */
				continue;
			}

			if( Widget->GetClass()->IsChildOf(WidgetClass) )
			{
				OutChildWidgets.Add(Widget);

				if( bEndSearchAsYouFind )
				{
					/* 一致したものがあればそのまま終了 */
					return (OutChildWidgets.Num() > 0);
				}
			}

			WidgetsToCheck.Push(Widget);
		}
	}

	return (OutChildWidgets.Num() > 0);
}

APlayerController* UGameDebugMenuWidget::GetOriginalPlayerController() const
{
	APlayerController* PlayerController = GetOwningPlayer();

	const ADebugCameraController* DCC = Cast<ADebugCameraController>(PlayerController);
	if (IsValid(DCC))
	{
		/* FLocalPlayerContextのGetPlayerControllerはADebugCameraControllerを無視できないのでオリジナルを返す */
		return DCC->OriginalControllerRef;
	}
	
	return PlayerController;
}
