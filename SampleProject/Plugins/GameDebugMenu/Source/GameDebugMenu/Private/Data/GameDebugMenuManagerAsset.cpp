/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Data/GameDebugMenuManagerAsset.h"

#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Input/GDMDebugCameraInput.h"

UGameDebugMenuManagerAsset::UGameDebugMenuManagerAsset()
	: Super()
	, DebugMenuRootWidgetClass()
	, DebugMenuClasses()
	, DebugMenuRegistrationOrder()
	, RootWidgetZOrder(TNumericLimits<int32>::Max() - 100)/* モバイルとかの仮想パッドより上になるよう強制で10加算されるので問題内容100小さくしてる。UGameViewportSubsystem::AddToScreen参照 */
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
