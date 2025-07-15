/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Data/GDMConsoleCommandSetAsset.h"

#include "GameDebugMenuSettings.h"

void UGDMConsoleCommandSetAsset::SetupCommandNames()
{
#if WITH_EDITORONLY_DATA
	const UGameDebugMenuSettings* Settings = GetDefault<UGameDebugMenuSettings>();

	auto FilterByCategory = [this](const auto& CommandArray, auto& OutArray)
	{
		OutArray.Reset();

		if (CategoryIndexList.IsEmpty())
		{
			for (const auto& Item : CommandArray)
			{
				OutArray.Add(Item);
			}
		}
		else
		{
			for (const auto& Item : CommandArray)
			{
				if (CategoryIndexList.Contains(Item.CategoryIndex))
				{
					OutArray.Add(Item);
				}
			}
		}
	};

	FilterByCategory(Settings->ConsoleCommandNames, ConsoleCommandNames);
	FilterByCategory(Settings->ConsoleCommandGroups, ConsoleCommandGroups);
	FilterByCategory(Settings->ConsoleCommandPairs, ConsoleCommandPairs);
	FilterByCategory(Settings->ConsoleCommandNumbers, ConsoleCommandNumbers);
	FilterByCategory(Settings->EditorOnlyConsoleCommandNames, EditorOnlyConsoleCommandNames);
	FilterByCategory(Settings->EditorOnlyConsoleCommandGroups, EditorOnlyConsoleCommandGroups);
	FilterByCategory(Settings->EditorOnlyConsoleCommandPairs, EditorOnlyConsoleCommandPairs);
	FilterByCategory(Settings->EditorOnlyConsoleCommandNumbers, EditorOnlyConsoleCommandNumbers);

	this->Modify();
#endif
}

void UGDMConsoleCommandSetAsset::SetupOrderConsoleCommandCategoryTitles()
{
#if WITH_EDITORONLY_DATA
	const UGameDebugMenuSettings* Settings = GetDefault<UGameDebugMenuSettings>();
	OrderConsoleCommandCategoryTitles = Settings->OrderConsoleCommandCategoryTitles;
	this->Modify();
#endif
}

void UGDMConsoleCommandSetAsset::MergeFromSourceAssets()
{
#if WITH_EDITORONLY_DATA
	auto MergeCommands = [](auto& Target, const auto& Source)
	{
		for (const auto& Item : Source)
		{
			const FString NewId = Item.BuildCommandIdentifier();
			if (!Target.ContainsByPredicate([&](const auto& Existing) { return Existing.BuildCommandIdentifier() == NewId; }))
			{
				Target.Add(Item);
			}
		}
	};
	
	for (const UGDMConsoleCommandSetAsset* SourceAsset : MergeSourceAssets)
	{
		if (!IsValid(SourceAsset))
		{
			continue;
		}

		MergeCommands(ConsoleCommandNames, SourceAsset->ConsoleCommandNames);
		MergeCommands(ConsoleCommandGroups, SourceAsset->ConsoleCommandGroups);
		MergeCommands(ConsoleCommandPairs, SourceAsset->ConsoleCommandPairs);
		MergeCommands(ConsoleCommandNumbers, SourceAsset->ConsoleCommandNumbers);

		MergeCommands(EditorOnlyConsoleCommandNames, SourceAsset->EditorOnlyConsoleCommandNames);
		MergeCommands(EditorOnlyConsoleCommandGroups, SourceAsset->EditorOnlyConsoleCommandGroups);
		MergeCommands(EditorOnlyConsoleCommandPairs, SourceAsset->EditorOnlyConsoleCommandPairs);
		MergeCommands(EditorOnlyConsoleCommandNumbers, SourceAsset->EditorOnlyConsoleCommandNumbers);
	}

	this->Modify();
#endif
}

TArray<FGDMMenuCategoryKey> UGDMConsoleCommandSetAsset::GetOrderConsoleCommandCategoryTitle() const
{
	TArray<FGDMMenuCategoryKey> ReturnValues;
	const auto& TitleArray = OrderConsoleCommandCategoryTitles;
	for( int32 Index = 0;Index < TitleArray.Num();++Index )
	{
		ReturnValues.Add(FGDMMenuCategoryKey(TitleArray[Index].Index, TitleArray[Index].Title));
	}

	return ReturnValues;
}


bool UGDMConsoleCommandSetAsset::GetConsoleCommandNameByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandSingle& Out) const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandSingle> Commands;
	Commands.Append(ConsoleCommandNames);
	Commands.Append(EditorOnlyConsoleCommandNames);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = ConsoleCommandNames;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGDMConsoleCommandSetAsset::GetNumConsoleCommandNames() const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandSingle> Commands;
	Commands.Append(ConsoleCommandNames);
	Commands.Append(EditorOnlyConsoleCommandNames);
	return Commands.Num();
#else
	return ConsoleCommandNames.Num();
#endif
}

bool UGDMConsoleCommandSetAsset::GetConsoleCommandGroupByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandGroup& Out) const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandGroup> Commands;
	Commands.Append(ConsoleCommandGroups);
	Commands.Append(EditorOnlyConsoleCommandGroups);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = ConsoleCommandGroups;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGDMConsoleCommandSetAsset::GetNumConsoleCommandGroups() const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandGroup> Commands;
	Commands.Append(ConsoleCommandGroups);
	Commands.Append(EditorOnlyConsoleCommandGroups);
	return Commands.Num();
#else
	return ConsoleCommandGroups.Num();
#endif
}

bool UGDMConsoleCommandSetAsset::GetConsoleCommandPairByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandPair& Out) const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandPair> Commands;
	Commands.Append(ConsoleCommandPairs);
	Commands.Append(EditorOnlyConsoleCommandPairs);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = ConsoleCommandPairs;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGDMConsoleCommandSetAsset::GetNumConsoleCommandPairs() const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandPair> Commands;
	Commands.Append(ConsoleCommandPairs);
	Commands.Append(EditorOnlyConsoleCommandPairs);
	return Commands.Num();
#else
	return ConsoleCommandPairs.Num();
#endif
}

bool UGDMConsoleCommandSetAsset::GetConsoleCommandNumberByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandNumber& Out) const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandNumber> Commands;
	Commands.Append(ConsoleCommandNumbers);
	Commands.Append(EditorOnlyConsoleCommandNumbers);
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#else
	const auto& Commands = ConsoleCommandNumbers;
	if (!Commands.IsValidIndex(ArrayIndex))
	{
		return false;
	}

	Out = Commands[ArrayIndex];
#endif
	return true;
}

int32 UGDMConsoleCommandSetAsset::GetNumConsoleCommandNumbers() const
{
#if WITH_EDITOR
	TArray<FGDMConsoleCommandNumber> Commands;
	Commands.Append(ConsoleCommandNumbers);
	Commands.Append(EditorOnlyConsoleCommandNumbers);
	return Commands.Num();
#else
	return ConsoleCommandNumbers.Num();
#endif
}

bool UGDMConsoleCommandSetAsset::FindConsoleCommandSingleById(const FString& CommandId, FGDMConsoleCommandSingle& Out) const
{
	for (const FGDMConsoleCommandSingle& Cmd : ConsoleCommandNames)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}

#if WITH_EDITOR
	for (const FGDMConsoleCommandSingle& Cmd : EditorOnlyConsoleCommandNames)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}
#endif

	return false;
}

bool UGDMConsoleCommandSetAsset::FindConsoleCommandGroupById(const FString& CommandId, FGDMConsoleCommandGroup& Out) const
{
	for (const FGDMConsoleCommandGroup& Cmd : ConsoleCommandGroups)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}

#if WITH_EDITOR
	for (const FGDMConsoleCommandGroup& Cmd : EditorOnlyConsoleCommandGroups)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}
#endif

	return false;
}

bool UGDMConsoleCommandSetAsset::FindConsoleCommandPairById(const FString& CommandId, FGDMConsoleCommandPair& Out) const
{
	for (const FGDMConsoleCommandPair& Cmd : ConsoleCommandPairs)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}

#if WITH_EDITOR
	for (const FGDMConsoleCommandPair& Cmd : EditorOnlyConsoleCommandPairs)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}
#endif

	return false;
}

bool UGDMConsoleCommandSetAsset::FindConsoleCommandNumberById(const FString& CommandId, FGDMConsoleCommandNumber& Out) const
{
	for (const FGDMConsoleCommandNumber& Cmd : ConsoleCommandNumbers)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}

#if WITH_EDITOR
	for (const FGDMConsoleCommandNumber& Cmd : EditorOnlyConsoleCommandNumbers)
	{
		if (Cmd.BuildCommandIdentifier() == CommandId)
		{
			Out = Cmd;
			return true;
		}
	}
#endif

	return false;
}
