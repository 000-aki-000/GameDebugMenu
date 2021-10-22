/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMButton.h"
#include <GameDebugMenuFunctions.h>
#include <GameDebugMenuManager.h>
#include <Components/ButtonSlot.h>
#include <Components/Button.h>

#define LOCTEXT_NAMESPACE "UMG"



UGDMButton::UGDMButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	IsFocusable = false;
}

#if WITH_EDITOR

const FText UGDMButton::GetPaletteCategory()
{
	return LOCTEXT("GDM", "GDM");
}

#endif

TSharedRef<SWidget> UGDMButton::RebuildWidget()
{
	MyButton = SNew(SButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, GDMSlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, GDMSlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, GDMSlateHandleReleased))
		.OnHovered_UObject(this, &ThisClass::GDMSlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::GDMSlateHandleUnhovered)
		.ButtonStyle(&WidgetStyle)
		.ClickMethod(ClickMethod)
		.TouchMethod(TouchMethod)
		.IsFocusable(IsFocusable)
		;

	if(GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}

	return MyButton.ToSharedRef();
}

FReply UGDMButton::GDMSlateHandleClicked()
{
	if(AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return FReply::Unhandled();
		}
	}
	return SlateHandleClicked();
}

void UGDMButton::GDMSlateHandlePressed()
{
	if(AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return;
		}
	}
	SlateHandlePressed();
}

void UGDMButton::GDMSlateHandleReleased()
{
	if(AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return;
		}
	}
	SlateHandleReleased();
}

void UGDMButton::GDMSlateHandleHovered()
{
	if(AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return;
		}
	}
	SlateHandleHovered();
}

void UGDMButton::GDMSlateHandleUnhovered()
{
	if(AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return;
		}
	}
	SlateHandleUnhovered();
}

#undef LOCTEXT_NAMESPACE
