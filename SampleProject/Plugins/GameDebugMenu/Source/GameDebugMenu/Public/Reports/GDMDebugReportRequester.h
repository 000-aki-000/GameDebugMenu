/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebImage.h"
#include "GDMDebugReportRequester.generated.h"

class AGameDebugMenuManager;
/**
 * バグレポート送信処理を行うクラス
 */
UCLASS(notplaceable)
class GAMEDEBUGMENU_API AGDMDebugReportRequester : public AActor
{
	GENERATED_BODY()

public:
	static const FString LineBreak;

	bool bSendLogs;
	bool bSendScreenshotCapture;
	FString Subject;
	FString Description;
	int32 IssueCategoryIndex;
	int32 PriorityIndex;
	int32 AssigneeIndex;
	TArray<uint8> ScreenshotImageData;
	FDateTime ScreenshotCapturedDateTime;
	int32 TestCount;
	int32 MaxTestCount;
	bool bWasRequestSuccessful;

public:	
	AGDMDebugReportRequester();

public:
	virtual void StartRequest();

protected:
	virtual void SuccessRequest();

	virtual void FailedRequest();

	AGameDebugMenuManager* GetOwnerDebugMenuManager() const;

	FString GetSubject();

	FString GetDescription();

	UFUNCTION(BlueprintNativeEvent)
	FString PrefixSubjectString();
	
	UFUNCTION(BlueprintNativeEvent)
	FString PrefixDescriptionString();
	
	UFUNCTION(BlueprintNativeEvent)
	FString SuffixSubjectString();
	
	UFUNCTION(BlueprintNativeEvent)
	FString SuffixDescriptionString();
	
	/* multipart/form-data形式で送信する場合使用 */
	int32 GetUTF8StringSize(const FString& Text);

	FString MakeBoundaryString();

	void AddContentString(const FString& BoundaryKey, const FString& DataName, const FString& ContentType, const FString& SourceData, TArray<uint8>& OutSendData);

	void AddContent(const FString& BoundaryKey, const FString& DataName, const FString& ContentType, const TArray<uint8>& SourceData, TArray<uint8>& OutSendData);

	void AddEndContentString(const FString& BoundaryKey, TArray<uint8>& OutSendData);
};
