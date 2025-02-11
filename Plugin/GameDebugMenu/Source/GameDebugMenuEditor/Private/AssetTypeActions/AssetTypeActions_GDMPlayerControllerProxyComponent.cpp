/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "AssetTypeActions/AssetTypeActions_GDMPlayerControllerProxyComponent.h"
#include "GameDebugMenuEditor.h"
#include "Component/GDMPlayerControllerProxyComponent.h"

FText FAssetTypeActions_GDMPlayerControllerProxyComponent::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GDMPlayerControllerProxyComponent", "GDM Player Controller Proxy Component");
}

FColor FAssetTypeActions_GDMPlayerControllerProxyComponent::GetTypeColor() const
{
	return FColor(60, 60, 60);
}

UClass* FAssetTypeActions_GDMPlayerControllerProxyComponent::GetSupportedClass() const
{
	return UGDMPlayerControllerProxyComponent::StaticClass();
}

uint32 FAssetTypeActions_GDMPlayerControllerProxyComponent::GetCategories()
{
	return FGameDebugMenuEditorModule::GetAssetCategory();
}

bool FAssetTypeActions_GDMPlayerControllerProxyComponent::CanLocalize() const
{
	return false;
}

FText FAssetTypeActions_GDMPlayerControllerProxyComponent::GetAssetDescription(const FAssetData& AssetData) const
{
	return FText::FromString(FString(TEXT("GDM Player Controller Proxy Component")));
}