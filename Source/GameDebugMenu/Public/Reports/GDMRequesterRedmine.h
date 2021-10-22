/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Reports/GDMDebugReportRequester.h"
#include "GDMRequesterRedmine.generated.h"

/**
 * Redmineへのバグレポート送信処理
 */
UCLASS()
class GAMEDEBUGMENU_API AGDMRequesterRedmine : public AGDMDebugReportRequester
{
	GENERATED_BODY()

	FString TokenScreenshotCapture;
	FString TokenLog;

public:
	virtual void StartRequest() override;

protected:
	virtual void RequestUploadScreenshotCapture();
	virtual void RequestUploadLog();
	virtual void RequestIssues();
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	virtual void OnResponseReceivedUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	virtual void OnResponseReceivedLog(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
