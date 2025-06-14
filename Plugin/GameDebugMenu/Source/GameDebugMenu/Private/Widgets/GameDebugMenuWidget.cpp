/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuWidget.h"
#include <Blueprint/WidgetTree.h>
#include "Engine/DebugCameraController.h"
#include "GameDebugMenuManager.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuSettings.h"
#include "Input/GDMEnhancedInputComponent.h"

UGameDebugMenuWidget::UGameDebugMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnSendWidgetEventDispatcher()
	, bActivateMenu(false)
	, InputHandles()
{
}

void UGameDebugMenuWidget::InitializeInputComponent()
{
	/* PushInputComponentまでしてしまうので呼び出さない */
//	Super::InitializeInputComponent();

	if (IsValid(InputComponent))
	{
		return;
	}
	
	if ( APlayerController* Controller = GetOwningPlayer() )
	{
		const UClass* Class = GetDefault<UGameDebugMenuSettings>()->GetDebugMenuInputComponentClass();
		InputComponent = NewObject<UGDMEnhancedInputComponent>( Controller, Class, NAME_None, RF_Transient);
	}
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

UGDMEnhancedInputComponent* UGameDebugMenuWidget::GetMyInputComponent() const
{
	return Cast<UGDMEnhancedInputComponent>(InputComponent);
}

bool UGameDebugMenuWidget::RegisterDebugMenuWidgetInputFunction(const UInputAction* Action, const FName FunctionName, const ETriggerEvent TriggerEvent, UObject* FunctionObject)
{
	InitializeInputComponent();
	
	if (UGDMEnhancedInputComponent* InputComp = GetMyInputComponent())
	{
		if (!IsValid(FunctionObject))
		{
			FunctionObject = this;
		}

		InputHandles.Add(InputComp->BindDebugMenuAction(Action, TriggerEvent, FunctionObject, FunctionName).GetHandle());
		return true;
	}

	return false;
}

bool UGameDebugMenuWidget::RegisterDebugMenuWidgetInputEvent(const UInputAction* Action, FOnGameDebugMenuWidgetInputAction Callback, const ETriggerEvent TriggerEvent)
{
	InitializeInputComponent();
	
	if (UGDMEnhancedInputComponent* InputComp = GetMyInputComponent())
	{
		const FEnhancedInputActionEventBinding& Binding =
		InputComp->BindDebugMenuAction(
			Action,
			TriggerEvent,
			[WeakThis = TWeakObjectPtr<UGameDebugMenuWidget>(this), Callback](const FInputActionInstance& Instance)
			{
				if (WeakThis.IsValid() && Callback.IsBound())
				{
					Callback.Execute();
				}
			});

		InputHandles.Add(Binding.GetHandle());
		return true;
	}

	return false;
}

void UGameDebugMenuWidget::UnregisterDebugMenuWidgetInputs()
{
	if (UGDMEnhancedInputComponent* InputComp = GetMyInputComponent())
	{
		for (const auto Handle : InputHandles)
		{
			InputComp->RemoveBindingByHandle(Handle);
		}

		InputHandles.Reset();
	}
}

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

void UGameDebugMenuWidget::ActivateDebugMenu()
{
	if (bActivateMenu)
	{
		return;
	}

	bActivateMenu = true;

	UGameDebugMenuFunctions::RegisterInputComponentForGameDebugMenu(this, InputComponent);
	
	OnActivateDebugMenu();
}

void UGameDebugMenuWidget::DeactivateDebugMenu()
{
	if (!bActivateMenu)
	{
		return;
	}

	bActivateMenu = false;

	UGameDebugMenuFunctions::UnregisterInputComponentForGameDebugMenu(this, InputComponent);
	
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

