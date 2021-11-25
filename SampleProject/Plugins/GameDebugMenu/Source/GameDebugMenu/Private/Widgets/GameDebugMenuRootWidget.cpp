/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuRootWidget.h"
#include <Blueprint/WidgetTree.h>
#include "Widgets/GDMTextBlock.h"
#include "GDMListenerComponent.h"

void UGameDebugMenuRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGDMListenerComponent* ListenerComponent = DebugMenuManager->GetListenerComponent();
	ListenerComponent->OnChangeDebugMenuLanguageDispatcher.AddDynamic(this, &UGameDebugMenuRootWidget::OnChangeDebugMenuLanguage);
}

void UGameDebugMenuRootWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if(IsValid(DebugMenuManager))
	{
		UGDMListenerComponent* ListenerComponent = DebugMenuManager->GetListenerComponent();
		ListenerComponent->OnChangeDebugMenuLanguageDispatcher.RemoveDynamic(this, &UGameDebugMenuRootWidget::OnChangeDebugMenuLanguage);
	}
}

void UGameDebugMenuRootWidget::OnChangeDebugMenuLanguage(const FName& NewLanguageKey, const FName& OldLanguageKey)
{
	/* Rootに追加されてるTextBlock */
	TArray<UWidget*> ChildWidgets;
	GetWidgetChildrenOfClass(UGDMTextBlock::StaticClass(), ChildWidgets, false);

	for( auto ChildWidget : ChildWidgets )
	{
		if( UGDMTextBlock* TextBlock = Cast<UGDMTextBlock>(ChildWidget) )
		{
			if( !TextBlock->DebugMenuStringKey.IsEmpty() )
			{
				TextBlock->SetText(FText::FromString(TextBlock->DebugMenuStringKey));
			}
		}
	}

	/* Rootに追加されてるDebugMenuWidget */
	GetWidgetChildrenOfClass(UGameDebugMenuWidget::StaticClass(), ChildWidgets, false);

	for( auto ChildWidget : ChildWidgets )
	{
		if( UGameDebugMenuWidget* DebugMenuWidget = Cast<UGameDebugMenuWidget>(ChildWidget) )
		{
			DebugMenuWidget->OnChangeDebugMenuLanguage(NewLanguageKey, OldLanguageKey);
		}
	}

	OnChangeDebugMenuLanguageBP(NewLanguageKey, OldLanguageKey);
}

bool UGameDebugMenuRootWidget::GetWidgetChildrenOfClass(TSubclassOf<UWidget> WidgetClass, TArray<UWidget*>& OutChildWidgets, bool bEndSearchAsYouFind)
{
	OutChildWidgets.Empty();

	/* 現在チェックするWidget郡 */
	TInlineComponentArray<UWidget*> WidgetsToCheck;

	/* チェック済みWidget郡 */
	TInlineComponentArray<UWidget*> CheckedWidgets;

	/* 作業用 */
	TArray<UWidget*> WorkWidgets;

	WidgetsToCheck.Push(this);

	while( WidgetsToCheck.Num() > 0 )
	{
		UWidget* PossibleParent = WidgetsToCheck.Pop(false);

		if( CheckedWidgets.Contains(PossibleParent) )
		{
			/* チェック済み */
			continue;
		}

		CheckedWidgets.Add(PossibleParent);

		WorkWidgets.Reset();

		if( UUserWidget* UserWidget = Cast<UUserWidget>(PossibleParent) )
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