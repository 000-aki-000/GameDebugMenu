/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GDMGameplayCategoryKeyCustomization.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/STextComboBox.h"
#include "GameDebugMenuSettings.h"

#define LOCTEXT_NAMESPACE "GDMGameplayCategoryKeyCustomization"

/************************************************************************/
/* FGOAPStateKeyCustomization										*/
/************************************************************************/

TSharedRef<IPropertyTypeCustomization> FGDMGameplayCategoryKeyCustomization::MakeInstance()
{
	return MakeShareable(new FGDMGameplayCategoryKeyCustomization());
}

void FGDMGameplayCategoryKeyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	MakeGameplayCategoryNames();

	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);

	for( uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
	{
		const TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex);
		if( ChildHandle->GetProperty()->GetName() == TEXT("Index") )
		{
			IndexHandle = ChildHandle;

			int32 CategoryIndex = 0;
			ChildHandle->GetValue(CategoryIndex);

			ArrayIndex = GetArrayIndex(CategoryIndex);
 			break;
		}
	}

	if( ArrayIndex >= GameplayCategoryNames.Num() )
	{
		ArrayIndex = 0;
	}

	check(IndexHandle.IsValid());

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SAssignNew(TextComboBox, STextComboBox)
					.OptionsSource(&GameplayCategoryNames)
					.OnSelectionChanged(this, &FGDMGameplayCategoryKeyCustomization::OnSelectionChanged)
					.InitiallySelectedItem(GameplayCategoryNames[ArrayIndex])
				]
			];
}

void FGDMGameplayCategoryKeyCustomization::MakeGameplayCategoryNames()
{
	GameplayCategoryNames.Reset();

	UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();
	for( int32 Index = 0; Index < Settings->OrderGameplayCategoryTitles.Num(); ++Index )
	{
		GameplayCategoryNames.Add(MakeShareable<FString>(new FString(Settings->GetGameplayCategoryTitle(Index))));
	}
}

void FGDMGameplayCategoryKeyCustomization::OnSelectionChanged(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo)
{
	if( ItemSelected.IsValid() )
	{
		for( int32 Index = 0; Index < GameplayCategoryNames.Num(); ++Index )
		{
			if( GameplayCategoryNames[Index] == ItemSelected )
			{
				ArrayIndex = Index;
				break;
			}
		}

		UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();
		IndexHandle->SetValue(Settings->GetGameplayCategoryIndex(ArrayIndex));
	}
}

int32 FGDMGameplayCategoryKeyCustomization::GetArrayIndex(int32 CategoryIndex)
{
	int32 ReturnValue = INDEX_NONE;
	UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();
	for( int32 Idx = 0; Idx < Settings->OrderGameplayCategoryTitles.Num(); ++Idx )
	{
		if( Settings->OrderGameplayCategoryTitles[Idx].Index == CategoryIndex )
		{
			ReturnValue = Idx;
			break;
		}
	}

	if( ReturnValue == INDEX_NONE )
	{
		ReturnValue = 0;
	}

	return ReturnValue;
}

#undef LOCTEXT_NAMESPACE