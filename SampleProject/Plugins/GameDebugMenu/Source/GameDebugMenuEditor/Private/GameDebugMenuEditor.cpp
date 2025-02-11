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
#include <PropertyEditorModule.h>
#include <EdGraphUtilities.h>

#include "GDMGameplayCategoryKeyCustomization.h"
#include "Pins/GDMGameplayCategoryKeyPinFactory.h"


#define LOCTEXT_NAMESPACE "FGameDebugMenuEditorModule"

EAssetTypeCategories::Type FGameDebugMenuEditorModule::GDMAssetCategory;

void FGameDebugMenuEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	GDMAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("GameDebugMenu")), LOCTEXT("GameDebugMenuAssetsCategory", "Game Debug Menu"));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GameDebugMenuManager()));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GDMPlayerControllerProxyComponent()));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_GameDebugMenuWidget()));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("GDMGameplayCategoryKey", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGDMGameplayCategoryKeyCustomization::MakeInstance));

	GameplayCategoryKeyPinFactory = MakeShareable(new FGDMGameplayCategoryKeyPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(GameplayCategoryKeyPinFactory);
}

void FGameDebugMenuEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("GDMGameplayCategoryKey");

	FEdGraphUtilities::UnregisterVisualPinFactory(GameplayCategoryKeyPinFactory);
	GameplayCategoryKeyPinFactory = nullptr;
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