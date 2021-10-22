/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenuTypes.h"


DEFINE_LOG_CATEGORY(LogGDM)


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
