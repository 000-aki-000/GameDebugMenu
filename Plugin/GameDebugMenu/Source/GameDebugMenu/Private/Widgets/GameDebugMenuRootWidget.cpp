/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuRootWidget.h"
#include "GameDebugMenuManager.h"
#include "Input/GDMInputSystemComponent.h"

void UGameDebugMenuRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	/* ルートは常にアクティブ状態に */
	bActivateMenu = true;
}

void UGameDebugMenuRootWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UGameDebugMenuRootWidget::ActivateDebugMenu()
{
	/* ルートは使用しない */
}

void UGameDebugMenuRootWidget::DeactivateDebugMenu()
{
	/* ルートは使用しない */
}

void UGameDebugMenuRootWidget::SetDebugMenuManager(AGameDebugMenuManager* InManager)
{
	Manager = InManager;
}

AGameDebugMenuManager* UGameDebugMenuRootWidget::GetOwnerManager() const
{
	return Manager;
}

void UGameDebugMenuRootWidget::SwitchInputComponentGroupForGameDebugMenu(const FName NewGroupName)
{
	Manager->GetDebugMenuInputSystemComponent()->SwitchToInputGroup(NewGroupName);
}