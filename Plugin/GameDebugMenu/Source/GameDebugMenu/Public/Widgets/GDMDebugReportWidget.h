/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuWidget.h"
#include "Reports/GDMDebugReportRequester.h"
#include "GDMDebugReportWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivedReportDelegate, bool, bWasSuccessful);

class UTexture2D;

UCLASS()
class GAMEDEBUGMENU_API UGDMDebugReportWidget : public UGameDebugMenuWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnReceivedReportDelegate OnReceivedReportDispatcher;

	UPROPERTY(BlueprintReadOnly, Transient)
	UTexture2D* ScreenshotTexture;

	UPROPERTY(BlueprintReadOnly, Transient)
	TArray<uint8> ScreenshotImageData;

	UPROPERTY(BlueprintReadWrite)
	int32 TestCount;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxTestCount;

protected:
	FDateTime ScreenshotCapturedDateTime;
	TWeakObjectPtr<AGDMDebugReportRequester> CurrentDebugReportRequester;

public:
	UGDMDebugReportWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	virtual void SendDebugReport(const FString& Subject, const FString& Description,int32 IssueCategoryIndex, int32 PriorityIndex, int32 AssigneeIndex, bool bSendLogs, bool bSendScreenshotCapture);

	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetIssueCategoryNameList();

	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetPriorityNameList();

	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetAssigneeNameList();

	UFUNCTION(BlueprintPure)
	virtual int32 GetDefaultIssueCategoryIndex();

	UFUNCTION(BlueprintPure)
	virtual int32 GetDefaultPriorityIndex();

	virtual void OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCreatedScreenshotTexture(UTexture2D* NewScreenshotTexture);

	UFUNCTION()
	virtual void OnRequesterDestroyed(AActor* DestroyedActor);

	UFUNCTION(BlueprintPure)
	virtual bool IsRequesting();
};
