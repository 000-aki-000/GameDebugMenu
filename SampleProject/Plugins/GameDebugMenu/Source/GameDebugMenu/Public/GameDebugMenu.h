/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Modules/ModuleManager.h"

class GAMEDEBUGMENU_API FGameDebugMenuModule : public IModuleInterface
{
public:
	/* Begin IModuleInterface */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/* End IModuleInterface */
};
