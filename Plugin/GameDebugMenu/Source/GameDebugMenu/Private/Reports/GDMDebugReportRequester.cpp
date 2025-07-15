/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Reports/GDMDebugReportRequester.h"
#include <GameDebugMenuManager.h>
#include <GameDebugMenuFunctions.h>
#include "Kismet/GameplayStatics.h"

const FString AGDMDebugReportRequester::LineBreak = TEXT("\r\n");

AGDMDebugReportRequester::AGDMDebugReportRequester()
	: Super()
	, bSendLogs(false)
	, bSendScreenshotCapture(false)
	, Subject()
	, Description()
	, IssueCategoryIndex(0)
	, PriorityIndex(0)
	, AssigneeIndex(0)
	, ScreenshotImageData()
	, ScreenshotCapturedDateTime()
	, TestCount(0)
	, MaxTestCount(0)
	, bWasRequestSuccessful(false)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGDMDebugReportRequester::StartRequest()
{
	bWasRequestSuccessful = false;
}

void AGDMDebugReportRequester::SuccessRequest()
{
	bWasRequestSuccessful = true;
	Destroy();
}

void AGDMDebugReportRequester::FailedRequest()
{
	bWasRequestSuccessful = false;
	Destroy();
}

AGameDebugMenuManager* AGDMDebugReportRequester::GetOwnerDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

FString AGDMDebugReportRequester::GetSubject()
{
	return PrefixSubjectString() + Subject + SuffixSubjectString();
}

FString AGDMDebugReportRequester::GetDescription()
{
	return PrefixDescriptionString() + Description + SuffixDescriptionString();
}

FString AGDMDebugReportRequester::PrefixSubjectString_Implementation()
{
	return FString();
}

FString AGDMDebugReportRequester::PrefixDescriptionString_Implementation()
{
	return FString();
}

FString AGDMDebugReportRequester::SuffixSubjectString_Implementation()
{
	return FString();
}

FString AGDMDebugReportRequester::SuffixDescriptionString_Implementation()
{
	return FString::Printf(TEXT("\n\n====================\n  Platform Name : %s\n  Build Version : %s\n  Build Configuration : %s\n  Project Version : %s\n  Test Count %d / %d\n")
					, *UGameplayStatics::GetPlatformName()
					, *UGameDebugMenuFunctions::GetGDMBuildVersionString()
					, *UGameDebugMenuFunctions::GetGDMBuildConfigurationString()
					, *UGameDebugMenuFunctions::GetGDMProjectVersionString()
					, TestCount, MaxTestCount
	);
}

int32 AGDMDebugReportRequester::GetUTF8StringSize(const FString& Text)
{
	int32 Size = 0;

	for(const TCHAR Char : Text)
	{
		const uint8 Code = static_cast<uint8>(*TCHAR_TO_UTF8(*FString::Chr(Char)));

		if((Code >= 0x00) && (Code <= 0x7f))
		{
			Size += 1;
		}
		else if((Code >= 0xc2) && (Code <= 0xdf))
		{
			Size += 2;
		}
		else if((Code >= 0xe0) && (Code <= 0xef))
		{
			Size += 3;
		}
		else if((Code >= 0xf0) && (Code <= 0xf7))
		{
			Size += 4;
		}
		else if((Code >= 0xf8) && (Code <= 0xfb))
		{
			Size += 5;
		}
		else if((Code >= 0xfc) && (Code <= 0xfd))
		{
			Size += 6;
		}
	}

	return Size;
}

FString AGDMDebugReportRequester::MakeBoundaryString()
{
	return FString::Printf(TEXT("ReportBoundary_%lld"), FDateTime::Now().ToUnixTimestamp());
}

void AGDMDebugReportRequester::AddContentString(const FString& BoundaryKey, const FString& DataName, const FString& ContentType, const FString& SourceData, TArray<uint8>& OutSendData)
{
	FString WorkStr = LineBreak + TEXT("--") + BoundaryKey + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	WorkStr = TEXT("Content-Disposition: form-data; ") + DataName + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	WorkStr = TEXT("Content-Type: ") + ContentType + LineBreak + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*SourceData), GetUTF8StringSize(SourceData));
}

void AGDMDebugReportRequester::AddContent(const FString& BoundaryKey, const FString& DataName, const FString& ContentType, const TArray<uint8>& SourceData, TArray<uint8>& OutSendData)
{
	FString WorkStr = LineBreak + TEXT("--") + BoundaryKey + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	WorkStr = TEXT("Content-Disposition: form-data; ") + DataName + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	WorkStr = TEXT("Content-Type: ") + ContentType + LineBreak + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));

	OutSendData.Append(SourceData);
}

void AGDMDebugReportRequester::AddEndContentString(const FString& BoundaryKey, TArray<uint8>& OutSendData)
{
	FString WorkStr = LineBreak + TEXT("--") + BoundaryKey + TEXT("--") + LineBreak;
	OutSendData.Append((uint8*)TCHAR_TO_UTF8(*WorkStr), GetUTF8StringSize(WorkStr));
}
