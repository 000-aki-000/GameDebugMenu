/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "SlateBasics.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "EdGraphUtilities.h"
#include "GDMGameplayCategoryKeyPin.h"

class FGDMGameplayCategoryKeyPinFactory : public FGraphPanelPinFactory
{
	/* 登録したオブジェクトなら自作ピンを返す */
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* InPin) const override
	{
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

		if (InPin->PinType.PinSubCategoryObject == FGDMGameplayCategoryKey::StaticStruct())
		{
			return SNew(SGDMGameplayCategoryKeyPin, InPin);
		}

		return nullptr;
	}
};