/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Widgets/GDMDebugReportWidget.h"
#include "GameDebugMenuSettings.h"
#include "Reports/GDMRequesterRedmine.h"
#include "Reports/GDMRequesterTrello.h"
#include "GameDebugMenuManager.h"
#include "GameDebugMenuFunctions.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"

UGDMDebugReportWidget::UGDMDebugReportWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OnReceivedReportDispatcher()
	, ScreenshotTexture(nullptr)
	, ScreenshotImageData()
	, TestCount(0)
	, MaxTestCount(0)
	, ScreenshotCapturedDateTime()
	, CurrentDebugReportRequester(nullptr)
{
}

void UGDMDebugReportWidget::SendDebugReport(const FString& Subject, const FString& Description, int32 IssueCategoryIndex, int32 PriorityIndex, int32 AssigneeIndex,bool bSendLogs, bool bSendScreenshotCapture)
{
	AGameDebugMenuManager* DebugMenuManager = UGameDebugMenuFunctions::GetGameDebugMenuManager(this);
	check(DebugMenuManager != nullptr);

	const TSubclassOf<AGDMDebugReportRequester>* RequesterClass = GetDefault<UGameDebugMenuSettings>()->GetDebugReportRequesterClass();
	if( RequesterClass == nullptr )
	{
		OnReceivedReportDispatcher.Broadcast(false);
		return;
	}

	CurrentDebugReportRequester = GetWorld()->SpawnActor<AGDMDebugReportRequester>(*RequesterClass);
	CurrentDebugReportRequester->SetOwner(DebugMenuManager);
	
	if (CurrentDebugReportRequester.IsValid())
	{
		CurrentDebugReportRequester->bSendLogs                  = bSendLogs;
		CurrentDebugReportRequester->bSendScreenshotCapture     = bSendScreenshotCapture;
		CurrentDebugReportRequester->Subject                    = Subject;
		CurrentDebugReportRequester->Description                = Description;
		CurrentDebugReportRequester->IssueCategoryIndex         = IssueCategoryIndex;
		CurrentDebugReportRequester->PriorityIndex              = PriorityIndex;
		CurrentDebugReportRequester->AssigneeIndex				= AssigneeIndex;
		CurrentDebugReportRequester->ScreenshotImageData        = ScreenshotImageData;
		CurrentDebugReportRequester->ScreenshotCapturedDateTime = ScreenshotCapturedDateTime;
		CurrentDebugReportRequester->TestCount                  = TestCount;
		CurrentDebugReportRequester->MaxTestCount               = MaxTestCount;
		CurrentDebugReportRequester->OnDestroyed.AddDynamic(this, &UGDMDebugReportWidget::OnRequesterDestroyed);
		CurrentDebugReportRequester->StartRequest();
	}
}

TArray<FText> UGDMDebugReportWidget::GetIssueCategoryNameList()
{
	return GetDefault<UGameDebugMenuSettings>()->GetIssueCategoryNameList();
}

TArray<FText> UGDMDebugReportWidget::GetPriorityNameList()
{
	return GetDefault<UGameDebugMenuSettings>()->GetPriorityNameList();
}

TArray<FText> UGDMDebugReportWidget::GetAssigneeNameList()
{
	return GetDefault<UGameDebugMenuSettings>()->GetAssigneeNameList();
}

int32 UGDMDebugReportWidget::GetDefaultIssueCategoryIndex()
{
	return GetDefault<UGameDebugMenuSettings>()->GetDefaultIssueCategoryIndex();
}

int32 UGDMDebugReportWidget::GetDefaultPriorityIndex()
{
	return GetDefault<UGameDebugMenuSettings>()->GetDefaultPriorityIndex();
}

void UGDMDebugReportWidget::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
	/* Debugレポートようにキャッシュ */
	ScreenshotImageData.Reset();
	const TArray<FColor> BitmapCopy(Bitmap);
	FImageUtils::ThumbnailCompressImageArray(Width, Height, BitmapCopy, ScreenshotImageData);

	UTexture2D* NewTexture = FImageUtils::ImportBufferAsTexture2D(ScreenshotImageData);
	NewTexture->SRGB = true;
	
	// /* テクスチャを生成 */
	// UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, EPixelFormat::PF_B8G8R8A8);
	// NewTexture->SRGB = true;
	//
	// /* テクスチャをコピーする */
	// void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	// const int32 TextureDataSize = Bitmap.Num() * 4;
	// FMemory::Memcpy(TextureData, Bitmap.GetData(), TextureDataSize);
	// NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	//
	// /* 更新 */
	// NewTexture->UpdateResource();

	ScreenshotTexture = NewTexture;

	ScreenshotCapturedDateTime = FDateTime::Now();

	OnCreatedScreenshotTexture(ScreenshotTexture);
}

void UGDMDebugReportWidget::OnRequesterDestroyed(AActor* DestroyedActor)
{
	AGDMDebugReportRequester* Requester = Cast<AGDMDebugReportRequester>(DestroyedActor);
	if (IsValid(Requester))
	{
		OnReceivedReportDispatcher.Broadcast(Requester->bWasRequestSuccessful);
	}
	else
	{
		OnReceivedReportDispatcher.Broadcast(false);
	}

	CurrentDebugReportRequester = nullptr;
}

bool UGDMDebugReportWidget::IsRequesting()
{
	return CurrentDebugReportRequester.IsValid();
}
