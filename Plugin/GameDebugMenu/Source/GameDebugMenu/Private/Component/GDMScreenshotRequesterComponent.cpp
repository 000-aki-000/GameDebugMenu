/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMScreenshotRequesterComponent.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "TimerManager.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuFunctions.h"
#include "Widgets/GDMDebugReportWidget.h"

UGDMScreenshotRequesterComponent::UGDMScreenshotRequesterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnScreenshotCapturedHandle()
	, OnScreenshotRequestProcessedHandle()
	, bRequestProcessed(true)
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

	bRequestProcessed = false;

	OnScreenshotCapturedHandle         = UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UGDMScreenshotRequesterComponent::OnScreenshotCaptured);
	OnScreenshotRequestProcessedHandle = FScreenshotRequest::OnScreenshotRequestProcessed().AddUObject(this, &UGDMScreenshotRequesterComponent::OnScreenshotRequestProcessed);

	GetOwnerGameDebugMenuManager()->CallStartScreenshotRequestDispatcher();

	/* ↑実際のキャプチャ処理は１フレーム後 */
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]()
	{
		/* スクショ開始 */
		FScreenshotRequest::Reset();
		FScreenshotRequest::RequestScreenshot(true);
	}));
}

bool UGDMScreenshotRequesterComponent::IsRequestProcessed()
{
	return bRequestProcessed;
}

AGameDebugMenuManager* UGDMScreenshotRequesterComponent::GetOwnerGameDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

void UGDMScreenshotRequesterComponent::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("OnScreenshotCaptured"), 4.0f);

	if(AGameDebugMenuManager* DebugMenuManager = GetOwnerGameDebugMenuManager())
	{
		TArray<UGameDebugMenuWidget*> Widgets;
		DebugMenuManager->GetDebugMenuWidgetInstances(Widgets);

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

	UGameDebugMenuFunctions::PrintLogScreen(this, TEXT("OnScreenshotRequestProcessed"), 4.0f);

	bRequestProcessed = true;

	GetOwnerGameDebugMenuManager()->CallScreenshotRequestProcessedDispatcher();
}

void UGDMScreenshotRequesterComponent::ResetHandle()
{
	UGameViewportClient::OnScreenshotCaptured().Remove(OnScreenshotCapturedHandle);
	FScreenshotRequest::OnScreenshotRequestProcessed().Remove(OnScreenshotRequestProcessedHandle);
	OnScreenshotCapturedHandle.Reset();
	OnScreenshotRequestProcessedHandle.Reset();
}