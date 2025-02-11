/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Factory/GameDebugMenuWidgetFactory.h"

#include "WidgetBlueprint.h"
#include "Widgets/GameDebugMenuWidget.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"

UGameDebugMenuWidgetFactory::UGameDebugMenuWidgetFactory(const class FObjectInitializer& Object)
	: Super(Object)
{
	SupportedClass = UGameDebugMenuWidget::StaticClass();
	bEditAfterNew  = true;
	bCreateNew     = true;
}

UObject* UGameDebugMenuWidgetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) 
{
	check(Class->IsChildOf(UGameDebugMenuWidget::StaticClass()));
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UWidgetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), TEXT("AssetTypeActions"));
	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
