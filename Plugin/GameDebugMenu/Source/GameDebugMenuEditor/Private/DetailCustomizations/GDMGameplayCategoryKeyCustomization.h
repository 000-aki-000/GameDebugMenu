/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/
#pragma once

#include "IPropertyTypeCustomization.h"
#include <Widgets/Input/STextComboBox.h>

class FGDMGameplayCategoryKeyCustomization : public IPropertyTypeCustomization
{
protected:
	TArray<TSharedPtr<FString>> GameplayCategoryNames;
	TSharedPtr<IPropertyHandle> IndexHandle;
	TSharedPtr<STextComboBox> TextComboBox;
	int32 ArrayIndex = 0;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/* Begin IPropertyTypeCustomization */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	/* End IPropertyTypeCustomization */

private:
	void MakeGameplayCategoryNames();
	void OnSelectionChanged(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
	int32 GetArrayIndex(int32 CategoryIndex);

};
