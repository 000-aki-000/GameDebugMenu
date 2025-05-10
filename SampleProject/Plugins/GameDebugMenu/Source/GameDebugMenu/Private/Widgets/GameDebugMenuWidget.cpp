/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuWidget.h"
#include <Blueprint/WidgetTree.h>
#include "GameDebugMenuManager.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Engine/DebugCameraController.h"
#include "EnhancedInputComponent.h"
#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuSettings.h"

UGameDebugMenuWidget::UGameDebugMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnSendWidgetEventDispatcher()
	, ParentGameDebugMenuWidget(nullptr)
	, bActivateMenu(false)
	, InputHandles()
{
}

void UGameDebugMenuWidget::InitializeInputComponent()
{
//	Super::InitializeInputComponent();

	if (IsValid(InputComponent))
	{
		return;
	}
	
	if ( APlayerController* Controller = GetOwningPlayer() )
	{
		InputComponent = NewObject<UInputComponent>( this, UEnhancedInputComponent::StaticClass(), TEXT("GameDebugMenuWidget_InputComponent"), RF_Transient);

		/* TODO ↓セッター経由だとInputComponentがないと更新できないんだが。。。。 */
		{
			/* 最後に追加されたUI用InputComponentだけ処理したい（アンバインド処理をしなくてもいいように）のでブロック指定にする */
			SetInputActionBlocking(true);

			/* DebugMenuはEnhancedInputを利用するので他のInputComponentより優先度は高くなるような値を指定 */
			SetInputActionPriority(GetDefault<UGameDebugMenuSettings>()->WidgetInputActionPriority);
		}
	}
}

UInputComponent* UGameDebugMenuWidget::GetMyInputComponent() const
{
	return InputComponent;
}

bool UGameDebugMenuWidget::RegisterDebugMenuWidgetInputFunction(const UInputAction* Action, const FName FunctionName, const ETriggerEvent TriggerEvent, UObject* FunctionObject)
{
	InitializeInputComponent();
	
	if (UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (!IsValid(FunctionObject))
		{
			FunctionObject = this;
		}

		InputHandles.Add(InputComp->BindAction(Action, TriggerEvent, FunctionObject, FunctionName).GetHandle());
		return true;
	}

	return false;
}

bool UGameDebugMenuWidget::RegisterDebugMenuWidgetInputEvent(const UInputAction* Action, FOnGameDebugMenuWidgetInputAction Callback, const ETriggerEvent TriggerEvent)
{
	InitializeInputComponent();
	
	if (UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		const FEnhancedInputActionEventBinding& Binding =
		InputComp->BindActionInstanceLambda(
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
	if (UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(InputComponent))
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
