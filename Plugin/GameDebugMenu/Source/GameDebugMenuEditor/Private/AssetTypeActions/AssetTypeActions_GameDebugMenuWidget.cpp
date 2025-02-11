/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "AssetTypeActions/AssetTypeActions_GameDebugMenuWidget.h"
#include "GameDebugMenuEditor.h"
#include "Widgets/GameDebugMenuWidget.h"

FText FAssetTypeActions_GameDebugMenuWidget::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GameDebugMenuWidget", "Game Debug Menu Widget");
}

FColor FAssetTypeActions_GameDebugMenuWidget::GetTypeColor() const
{
	return FColor(60, 60, 60);
}

UClass* FAssetTypeActions_GameDebugMenuWidget::GetSupportedClass() const
{
	return UGameDebugMenuWidget::StaticClass();
}

uint32 FAssetTypeActions_GameDebugMenuWidget::GetCategories()
{
	return FGameDebugMenuEditorModule::GetAssetCategory();
}

bool FAssetTypeActions_GameDebugMenuWidget::CanLocalize() const
{
	return false;
}

FText FAssetTypeActions_GameDebugMenuWidget::GetAssetDescription(const FAssetData& AssetData) const
{
	return FText::FromString(FString(TEXT("Game Debug Menu Widget")));
}