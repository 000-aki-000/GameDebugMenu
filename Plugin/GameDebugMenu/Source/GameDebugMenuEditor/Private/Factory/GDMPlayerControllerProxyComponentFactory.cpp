/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Factory/GDMPlayerControllerProxyComponentFactory.h"
#include "Component/GDMPlayerControllerProxyComponent.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"

UGDMPlayerControllerProxyComponentFactory::UGDMPlayerControllerProxyComponentFactory(const class FObjectInitializer& Object)
	: Super(Object)
{
	SupportedClass = UGDMPlayerControllerProxyComponent::StaticClass();
	bEditAfterNew  = true;
	bCreateNew     = true;
}

UObject* UGDMPlayerControllerProxyComponentFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UGDMPlayerControllerProxyComponent::StaticClass()));
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), TEXT("AssetTypeActions"));
	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
