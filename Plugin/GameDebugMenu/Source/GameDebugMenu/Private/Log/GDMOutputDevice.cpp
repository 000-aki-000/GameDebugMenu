/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Log/GDMOutputDevice.h"
#include "HAL/CriticalSection.h"
#include <CoreGlobals.h>
#include <Misc/OutputDeviceHelper.h>

#include "GameDebugMenuSettings.h"

FGDMOutputDevice::FGDMOutputDevice()
	: FOutputDevice()
	, Logs()
{
	Logs.Reserve(10000);
	CommandHistory.Reserve(100);
	GLog->AddOutputDevice(this);
}

FGDMOutputDevice::~FGDMOutputDevice()
{
	if(GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
}

void FGDMOutputDevice::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const class FName& Category, const double Time)
{
	this->Serialize(Data, Verbosity, Category);
}

void FGDMOutputDevice::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if(Verbosity == ELogVerbosity::SetColor)
	{
		// Skip Color Events
		return;
	}
	else
	{
		static FName CommandCategory = TEXT("Cmd");

		if (Category == CommandCategory)
		{
			while(CommandHistory.Num() > GetDefault<UGameDebugMenuSettings>()->MaxCommandHistoryNum - 1)
			{
				CommandHistory.RemoveAt(0);
			}

			bool bAdd = true;
			const FString CheckStr = Data;
			for (const auto& C : GetDefault<UGameDebugMenuSettings>()->NoSaveConsoleCommands)
			{
				if (CheckStr.Contains(C))
				{
					bAdd = false;
					break;
				}
			}

			if (bAdd)
			{
				CommandHistory.Add(Data);
			}
		}
		
		/* 時間はUTCで固定し他はエディターの「outputlog」のものと同じものを保持しとく */
		static ELogTimes::Type LogTimestampMode = ELogTimes::UTC;

		// handle multiline strings by breaking them apart by line
		TArray<FTextRange> LineRanges;
		FString CurrentLogDump = Data;
		FTextRange::CalculateLineRangesFromString(CurrentLogDump, LineRanges);

		bool bIsFirstLineInMessage = true;
		for(const FTextRange& LineRange : LineRanges)
		{
			if(!LineRange.IsEmpty())
			{
				FString Line = CurrentLogDump.Mid(LineRange.BeginIndex, LineRange.Len());
				Line = Line.ConvertTabsToSpaces(4);

				// Hard-wrap lines to avoid them being too long
				static const int32 HardWrapLen = 360;
				for(int32 CurrentStartIndex = 0; CurrentStartIndex < Line.Len();)
				{
					int32 HardWrapLineLen = 0;
					if(bIsFirstLineInMessage)
					{
						FString MessagePrefix = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, nullptr, LogTimestampMode);

						HardWrapLineLen = FMath::Min(HardWrapLen - MessagePrefix.Len(), Line.Len() - CurrentStartIndex);
						FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

						Logs.Add(MessagePrefix + HardWrapLine);
					}
					else
					{
						HardWrapLineLen = FMath::Min(HardWrapLen, Line.Len() - CurrentStartIndex);
						FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

						Logs.Add(MoveTemp(HardWrapLine));
					}

					bIsFirstLineInMessage = false;
					CurrentStartIndex += HardWrapLineLen;
				}
			}
		}
	}
}

TArray<FString> FGDMOutputDevice::GetLogs() const
{
	FScopeLock Lock(&CommandHistoryMutex);
	return Logs;
}

TArray<FString> FGDMOutputDevice::GetCommandHistory() const
{
	FScopeLock Lock(&CommandHistoryMutex);
	return CommandHistory;
}

void FGDMOutputDevice::ClearCommandHistory()
{
	FScopeLock Lock(&CommandHistoryMutex);
	CommandHistory.Reset();
}