/**
* Copyright (c) 2022 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuTypes.h"


DEFINE_LOG_CATEGORY(LogGDM)

/************************************************************************/
/* FGDMJiraSettings														*/
/************************************************************************/

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

/************************************************************************/
/* FGDMMenuCategoryKey													*/
/************************************************************************/

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

/************************************************************************/
/* FGDMGameplayCategoryKey												*/
/************************************************************************/

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
