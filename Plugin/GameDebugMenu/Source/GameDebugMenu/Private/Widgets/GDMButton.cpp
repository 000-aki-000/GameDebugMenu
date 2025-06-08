/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMButton.h"
#include "Widgets/Input/SButton.h"
#include <GameDebugMenuFunctions.h>
#include <GameDebugMenuManager.h>
#include "Components/ButtonSlot.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"



UGDMButton::UGDMButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitIsFocusable(false);

	/* デフォのDownAndUpだと別UserWidget上でボタンをクリックしたときに１度無視され２回クリックしないと応答しないことがあるため変更 */
	SetClickMethod(EButtonClickMethod::Type::MouseDown);
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
		.ButtonStyle(&GetStyle())
		.ClickMethod(GetClickMethod())
		.TouchMethod(GetTouchMethod())
		.IsFocusable(GetIsFocusable())
		;

	if(GetChildrenCount() > 0)
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}

	return MyButton.ToSharedRef();
}

FReply UGDMButton::GDMSlateHandleClicked()
{
	if(const AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
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
	if(const AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
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
	if(const AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
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
	if(const AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
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
	if(const AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this))
	{
		if(DebugMenuManager->IsInputIgnored())
		{
			return;
		}
	}
	SlateHandleUnhovered();
}

#undef LOCTEXT_NAMESPACE
