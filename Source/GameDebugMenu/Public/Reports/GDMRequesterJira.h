/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Reports/GDMDebugReportRequester.h"
#include "GDMRequesterJira.generated.h"

/**
 * Jiraへのバグレポート送信処理
 */
UCLASS()
class GAMEDEBUGMENU_API AGDMRequesterJira : public AGDMDebugReportRequester
{
	GENERATED_BODY()

public:
	virtual void StartRequest() override;

protected:
	virtual void RequestUploadContent(const FString& IssueKey);
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	virtual void OnResponseReceivedUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
