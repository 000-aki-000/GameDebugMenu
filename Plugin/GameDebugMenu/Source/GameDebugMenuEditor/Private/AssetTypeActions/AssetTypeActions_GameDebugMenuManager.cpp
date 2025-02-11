/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "AssetTypeActions/AssetTypeActions_GameDebugMenuManager.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuEditor.h"

FText FAssetTypeActions_GameDebugMenuManager::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GameDebugMenuManager", "Game Debug Menu Manager");
}

FColor FAssetTypeActions_GameDebugMenuManager::GetTypeColor() const
{
	return FColor(60, 60, 60);
}

UClass* FAssetTypeActions_GameDebugMenuManager::GetSupportedClass() const
{
	return AGameDebugMenuManager::StaticClass();
}

uint32 FAssetTypeActions_GameDebugMenuManager::GetCategories()
{
	return FGameDebugMenuEditorModule::GetAssetCategory();
}

bool FAssetTypeActions_GameDebugMenuManager::CanLocalize() const
{
	return false;
}

FText FAssetTypeActions_GameDebugMenuManager::GetAssetDescription(const FAssetData& AssetData) const
{
	return FText::FromString(FString(TEXT("Game Debug Menu Manager")));
}