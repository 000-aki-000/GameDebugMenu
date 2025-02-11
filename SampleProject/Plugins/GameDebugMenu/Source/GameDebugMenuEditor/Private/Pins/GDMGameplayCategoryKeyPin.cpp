/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GDMGameplayCategoryKeyPin.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Widgets/Input/STextComboBox.h"
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuTypes.h"


/************************************************************************/
/* SGoapStateKeyPin												*/
/************************************************************************/

void SGDMGameplayCategoryKeyPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SGDMGameplayCategoryKeyPin::GetDefaultValueWidget()
{
	MakeGameplayCategoryNames();

	ArrayIndex = GetCategoryNameArrayIndex();

	TextComboBox = SNew(STextComboBox)
					.Visibility(this, &SGDMGameplayCategoryKeyPin::GetStateKeyVisibility)
					.OptionsSource(&GameplayCategoryNames)
					.OnSelectionChanged(this, &SGDMGameplayCategoryKeyPin::OnSelectionChanged)
					.InitiallySelectedItem(GameplayCategoryNames[ArrayIndex]);

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		[
			TextComboBox.ToSharedRef()
		];
}

void SGDMGameplayCategoryKeyPin::MakeGameplayCategoryNames()
{
	GameplayCategoryNames.Empty();

	UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();
	for( int32 Index = 0; Index < Settings->OrderGameplayCategoryTitles.Num(); ++Index )
	{
		GameplayCategoryNames.Add(MakeShareable<FString>(new FString(Settings->GetGameplayCategoryTitle(Index))));
	}
}

int32 SGDMGameplayCategoryKeyPin::GetCategoryNameArrayIndex() const
{
	int32 GraphPinValue = 0;

	const FString CurrentDefault = GraphPinObj->GetDefaultAsString();
	if( CurrentDefault.Len() > 0 )
	{
		constexpr int32 StartIndex = 7;/* (Index= */
		int32 EndIndex;
		CurrentDefault.FindChar(',', EndIndex);
		const FString DefaultValString = CurrentDefault.Mid(StartIndex, EndIndex - StartIndex);
		GraphPinValue = FCString::Atoi(*DefaultValString);
	}

	int32 Index = 0;
	UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();
	for( const FGDMOrderMenuCategoryTitle& CategoryTitle : Settings->OrderGameplayCategoryTitles )
	{
		if( CategoryTitle.Index == GraphPinValue )
		{
			return Index;
		}
		Index++;
	}

	return 0;
}

void SGDMGameplayCategoryKeyPin::SetCategoryValue(const int32 InArrayIndex) const
{
	UGameDebugMenuSettings* Settings = GetMutableDefault<UGameDebugMenuSettings>();

	FString StrKey;
	StrKey.AppendInt(Settings->GetGameplayCategoryIndex(InArrayIndex));

	const FString NewValue = TEXT("(Index=") + StrKey + TEXT(",KeyName=)");

	GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, NewValue);
}

void SGDMGameplayCategoryKeyPin::OnSelectionChanged(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo)
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
	}

	SetCategoryValue(ArrayIndex);
}

EVisibility SGDMGameplayCategoryKeyPin::GetStateKeyVisibility() const
{
	return IsConnected() ? EVisibility::Collapsed : EVisibility::Visible;
}
