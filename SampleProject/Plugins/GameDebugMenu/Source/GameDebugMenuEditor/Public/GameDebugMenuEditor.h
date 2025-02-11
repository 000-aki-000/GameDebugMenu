/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Modules/ModuleManager.h"
#include <AssetTypeCategories.h>

class IAssetTools;
class IAssetTypeActions;

class FGDMGameplayCategoryKeyPinFactory;

class FGameDebugMenuEditorModule : public IModuleInterface
{
private:
	static EAssetTypeCategories::Type GDMAssetCategory;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
	TSharedPtr<FGDMGameplayCategoryKeyPinFactory> GameplayCategoryKeyPinFactory;

public:
	/* Begin IModuleInterface */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	/* End IModuleInterface */
	
public:
	static EAssetTypeCategories::Type GetAssetCategory();

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
};