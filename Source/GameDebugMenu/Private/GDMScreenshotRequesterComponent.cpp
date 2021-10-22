/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GDMScreenshotRequesterComponent.h"
#include "GameDebugMenuFunctions.h"
#include "Widgets/GDMDebugReportWidget.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "GameDebugMenuManager.h"

UGDMScreenshotRequesterComponent::UGDMScreenshotRequesterComponent()
	: Super()
	, OnScreenshotRequestProcessedDispatcher()
	, OnScreenshotCapturedHandle()
	, OnScreenshotRequestProcessedHandle()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGDMScreenshotRequesterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGDMScreenshotRequesterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ResetHandle();
}

void UGDMScreenshotRequesterComponent::RequestScreenshot()
{
	ResetHandle();

	OnScreenshotCapturedHandle         = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UGDMScreenshotRequesterComponent::OnScreenshotCaptured);
	OnScreenshotRequestProcessedHandle = FScreenshotRequest::OnScreenshotRequestProcessed().AddUObject(this, &UGDMScreenshotRequesterComponent::OnScreenshotRequestProcessed);

	/* スクショ開始 */
	FScreenshotRequest::Reset();
	FScreenshotRequest::RequestScreenshot(true);
}

AGameDebugMenuManager* UGDMScreenshotRequesterComponent::GetOwnerGameDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

void UGDMScreenshotRequesterComponent::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
	UGameDebugMenuFunctions::PrintLog(this, TEXT("OnScreenshotCaptured"), 4.0f);

	if(AGameDebugMenuManager* DebugMenuManager = GetOwnerGameDebugMenuManager())
	{
		TArray<UGameDebugMenuWidget*> Widgets;
		DebugMenuManager->GetDebugDetailScreenInstances(Widgets);

		for(const auto Widget : Widgets)
		{
			/* Debugレポート用Widgetがあれば通知してあげる */
			if(UGDMDebugReportWidget* ReportWidget = Cast<UGDMDebugReportWidget>(Widget))
			{
				ReportWidget->OnScreenshotCaptured(Width, Height, Bitmap);
			}
		}
	}
}

void UGDMScreenshotRequesterComponent::OnScreenshotRequestProcessed()
{
	ResetHandle();

	UGameDebugMenuFunctions::PrintLog(this, TEXT("OnScreenshotRequestProcessed"), 4.0f);

	OnScreenshotRequestProcessedDispatcher.Broadcast();
}

void UGDMScreenshotRequesterComponent::ResetHandle()
{
	UGameViewportClient::OnScreenshotCaptured().Remove(OnScreenshotCapturedHandle);
	FScreenshotRequest::OnScreenshotRequestProcessed().Remove(OnScreenshotRequestProcessedHandle);
	OnScreenshotCapturedHandle.Reset();
	OnScreenshotRequestProcessedHandle.Reset();
}