/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuEditor.h"
#include "ISettingsModule.h"
#include "GameDebugMenuSettings.h"
#include <AssetToolsModule.h>
#include "AssetTypeActions/AssetTypeActions_GameDebugMenuManager.h"
#include "AssetTypeActions/AssetTypeActions_GDMPlayerControllerProxyComponent.h"
#include "AssetTypeActions/AssetTypeActions_GameDebugMenuWidget.h"


#define LOCTEXT_NAMESPACE "FGameDebugMenuEditorModule"

EAssetTypeCategories::Type FGameDebugMenuEditorModule::GDMAssetCategory;

void FGameDebugMenuEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if( SettingsModule != nullptr )
	{
		SettingsModule->RegisterSettings("Project","Plugins","GameDebugMenu",
										 LOCTEXT("SettingsName","GameDebugMenu"),
										 LOCTEXT("SettingsDescription","Configure the GameDebugMenu Plugin"),
										 GetMutableDefault<UGameDebugMenuSettings>()
		);
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	GDMAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("GameDebugMenu")), LOCTEXT("GameDebugMenuAssetsCategory", "Game Debug Menu"));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GameDebugMenuManager()));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GDMPlayerControllerProxyComponent()));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GameDebugMenuWidget()));
}

void FGameDebugMenuEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if( SettingsModule != nullptr )
	{
		SettingsModule->UnregisterSettings("Project","GameDebugMenu","GameDebugMenuSettings");
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
}


EAssetTypeCategories::Type FGameDebugMenuEditorModule::GetAssetCategory()
{
	return GDMAssetCategory;
}

void FGameDebugMenuEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameDebugMenuEditorModule, GameDebugMenuEditor)