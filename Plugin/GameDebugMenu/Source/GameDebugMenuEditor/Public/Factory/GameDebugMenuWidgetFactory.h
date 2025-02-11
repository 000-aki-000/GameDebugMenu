/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Factories/Factory.h"
#include "GameDebugMenuWidgetFactory.generated.h"

#define LOCTEXT_NAMESPACE "GameDebugMenu"

/**
 * 
 */
UCLASS()
class GAMEDEBUGMENUEDITOR_API UGameDebugMenuWidgetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameDebugMenuWidgetFactory(const class FObjectInitializer& Object);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

#undef LOCTEXT_NAMESPACE
