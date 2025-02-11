/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GameDebugMenuRootWidget.h"

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

void UGameDebugMenuRootWidget::ActivateDebugMenu(bool bAlwaysExecute)
{
	/* ルートは使用しない */
}

void UGameDebugMenuRootWidget::DeactivateDebugMenu()
{
	/* ルートは使用しない */
}