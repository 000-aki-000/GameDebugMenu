/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Factory/GameDebugMenuManagerFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GameDebugMenuManager.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"

UGameDebugMenuManagerFactory::UGameDebugMenuManagerFactory(const class FObjectInitializer& Object) : Super(Object)
{
	SupportedClass = AGameDebugMenuManager::StaticClass();
	bEditAfterNew  = true;
	bCreateNew     = true;
}

UObject* UGameDebugMenuManagerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(AGameDebugMenuManager::StaticClass()));
	return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), TEXT("AssetTypeActions"));
}

#undef LOCTEXT_NAMESPACE
