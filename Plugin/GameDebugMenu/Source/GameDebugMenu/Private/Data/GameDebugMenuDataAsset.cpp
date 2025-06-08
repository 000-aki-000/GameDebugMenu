/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Data/GameDebugMenuDataAsset.h"

#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Input/GDMDebugCameraInput.h"

UGameDebugMenuDataAsset::UGameDebugMenuDataAsset()
	: Super()
	, DebugMenuRootWidgetClass()
	, DebugMenuClasses()
	, DebugMenuRegistrationOrder()
	, RootWidgetZOrder(TNumericLimits<int32>::Max())
	, AddInputMappingContextWhenCreateManager()
	, AddInputMappingContextWhenDebugMenuIsShow()
	, DebugCameraInputClass(nullptr)
	, FavoriteItemDefinitions()
	, DebugMenuPCProxyComponentClass()
	, bGamePause(false)
{
	DebugMenuPCProxyComponentClass = UGDMPlayerControllerProxyComponent::StaticClass();
	DebugCameraInputClass = AGDMDebugCameraInput::StaticClass();
}
