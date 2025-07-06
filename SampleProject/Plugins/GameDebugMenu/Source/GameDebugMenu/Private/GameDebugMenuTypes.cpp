/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuTypes.h"


DEFINE_LOG_CATEGORY(LogGDM)

/************************************************************************
 * FGDMConsoleCommand
 ************************************************************************/
FGDMConsoleCommand::FGDMConsoleCommand()
{
	CategoryIndex  = 255;
	Title          = FText::GetEmpty();
	Description    = FText::GetEmpty();
	Type           = EGDMConsoleCommandType::Non;
	ClickedEvent   = EGDMConsoleCommandClickedEvent::Non;
	CommandNetType = EGDMConsoleCommandNetType::LocalOnly;
}

FGDMConsoleCommandSingle::FGDMConsoleCommandSingle()
	: Super()
{
	ConsoleCommandName.Empty();
	Type = EGDMConsoleCommandType::Single;
}

FString FGDMConsoleCommandSingle::BuildCommandIdentifier() const
{
	return FString::Printf(TEXT("Single_%s"), *ConsoleCommandName.Replace(TEXT(" "), TEXT("_")));
}

FGDMConsoleCommandGroup::FGDMConsoleCommandGroup()
	: Super()
{
	ConsoleCommandNames.Reset();
	Type = EGDMConsoleCommandType::Group;
}

FString FGDMConsoleCommandGroup::BuildCommandIdentifier() const
{
	const FString Joined = FString::Join(ConsoleCommandNames, TEXT("_"));
	return FString::Printf(TEXT("Group_%s"), *Joined.Replace(TEXT(" "), TEXT("_")));
}

FGDMConsoleCommandPair::FGDMConsoleCommandPair()
	: Super()
{
	FirstConsoleCommandName.Empty();
	SecondConsoleCommandName.Empty();
	Type = EGDMConsoleCommandType::Pair;
}

FString FGDMConsoleCommandPair::BuildCommandIdentifier() const
{
	return FString::Printf(TEXT("Pair_%s_TO_%s"),
		*FirstConsoleCommandName.Replace(TEXT(" "), TEXT("_")),
		*SecondConsoleCommandName.Replace(TEXT(" "), TEXT("_"))
	);
}

FGDMConsoleCommandNumber::FGDMConsoleCommandNumber()
	: Super()
{
	ConsoleCommandName.Empty();
	PreConsoleCommandName.Empty();
	PostConsoleCommandName.Empty();
	Type                        = EGDMConsoleCommandType::Number;
	UIConfigInfo.Range.bUseMax  = true;
	UIConfigInfo.Range.bUseMin  = true;
	UIConfigInfo.Range.MinValue = 0.0f;
	UIConfigInfo.Range.MaxValue = 1.0f;
	DefaultValue                = 0.0f;
	ConsoleVariableName.Empty();
}

FString FGDMConsoleCommandNumber::BuildCommandIdentifier() const
{
	return FString::Printf(TEXT("Number_%s_Default%.2f"),
		*ConsoleCommandName.Replace(TEXT(" "), TEXT("_")),
		DefaultValue
	);
}

/************************************************************************
 * FGDMJiraSettings
 ************************************************************************/

FString FGDMJiraSettings::GetAssigneeAccountIdByListIndex(int32 ListIndex) const
{
	TArray<FString> AccountIds;
	AssigneeList.GenerateKeyArray(AccountIds);
	if(AccountIds.IsValidIndex(ListIndex))
	{
		return AccountIds[ListIndex];
	}
	return FString();
}

FText FGDMJiraSettings::GetAssigneeTextByListIndex(int32 ListIndex) const
{
	TArray<FText> TextList;
	AssigneeList.GenerateValueArray(TextList);
	if(TextList.IsValidIndex(ListIndex))
	{
		return TextList[ListIndex];
	}
	return FText();
}

/************************************************************************
 * FGDMMenuCategoryKey
 ************************************************************************/

FGDMMenuCategoryKey::FGDMMenuCategoryKey(uint8 InIndex, FString InKeyName)
	: Index(InIndex)
	, KeyName(InKeyName)
{
}

FGDMMenuCategoryKey::FGDMMenuCategoryKey(uint8 InIndex)
	: Index(InIndex)
	, KeyName()
{
}

FGDMMenuCategoryKey::FGDMMenuCategoryKey()
	: Index(0)
{
}

bool FGDMMenuCategoryKey::operator==(FGDMMenuCategoryKey& InOther)
{
	return InOther.Index == Index;
}

bool FGDMMenuCategoryKey::operator!=(FGDMMenuCategoryKey& InOther)
{
	return !((*this) == InOther);
}

bool FGDMMenuCategoryKey::operator<(FGDMMenuCategoryKey& InOther)
{
	return InOther.Index < Index;
}

bool FGDMMenuCategoryKey::operator>(FGDMMenuCategoryKey& InOther)
{
	return InOther.Index > Index;
}

/************************************************************************
 * FGDMGameplayCategoryKey
 ************************************************************************/

FGDMGameplayCategoryKey::FGDMGameplayCategoryKey(uint8 InKey, FString InKeyName)
	: Super(InKey,InKeyName)
{
}

FGDMGameplayCategoryKey::FGDMGameplayCategoryKey(uint8 InKey)
	: Super(InKey)
{
}

FGDMGameplayCategoryKey::FGDMGameplayCategoryKey()
	: Super()
{
}
