/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMPadInputWidgetController.h"
#include <Widgets/GameDebugMenuWidget.h>
#include "Engine/Engine.h"

UWorld* UGDMPadInputWidgetController::GetWorld() const
{
	if( !IsValid(OwnerGameDebugMenuWidget) )
	{
		return GWorld;
	}
	return OwnerGameDebugMenuWidget->GetWorld();
}
