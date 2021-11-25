/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "SlateBasics.h"
#include "SGraphPin.h"

/************************************************************************/
/* SGDMGameplayCategoryKeyPin											*/
/************************************************************************/

class SGDMGameplayCategoryKeyPin : public SGraphPin
{
	int32 ArrayIndex;
	TArray<TSharedPtr<FString>> GameplayCategoryNames;
	TSharedPtr<STextComboBox> TextComboBox;

public:
	SLATE_BEGIN_ARGS(SGDMGameplayCategoryKeyPin) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

private:
	void MakeGameplayCategoryNames();
	int32 GetCategoryNameArrayIndex();
	void SetCategoryValue(const int32 InArrayIndex);
	void OnSelectionChanged(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
	EVisibility GetStateKeyVisibility() const;
	void OnObjectPropertyChanged(UObject* Asset, FPropertyChangedEvent& PropertyChangedEvent);

};
