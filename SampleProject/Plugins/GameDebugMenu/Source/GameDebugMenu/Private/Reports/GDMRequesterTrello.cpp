/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Reports/GDMRequesterTrello.h"
#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "GameDebugMenuSettings.h"
#include <Kismet/KismetSystemLibrary.h>
#include "GameDebugMenuManager.h"

void AGDMRequesterTrello::StartRequest()
{
	bWasRequestSuccessful = false;
	const FGDMTrelloSettings& TrelloSettings = GetDefault<UGameDebugMenuSettings>()->TrelloSettings;
	AttachmentCardListID.Reset();

	FString URL(TEXT("https://api.trello.com/1/cards?"));
	URL += TEXT("?pos=top");
	URL += TEXT("&idList=");
	URL += TrelloSettings.CardListIDs[IssueCategoryIndex];
	URL += TEXT("&key=");
	URL += TrelloSettings.AccessKey;
	URL += TEXT("&token=");
	URL += TrelloSettings.AccessToken;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterTrello::OnResponseReceived);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));

	const FString BoundaryKey = MakeBoundaryString();
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *BoundaryKey));

	TArray<uint8> ContentData;
	AddContentString(BoundaryKey, TEXT("name=\"name\";"), TEXT("text/plain; charset=UTF-8"), GetSubject(), ContentData);
	AddContentString(BoundaryKey, TEXT("name=\"desc\";"), TEXT("text/plain; charset=UTF-8"), GetDescription(), ContentData);
	AddEndContentString(BoundaryKey, ContentData);

	Request->SetContent(ContentData);
	Request->ProcessRequest();
}

void AGDMRequesterTrello::RequestUploadScreenshotCapture()
{
	const FGDMTrelloSettings& TrelloSettings = GetDefault<UGameDebugMenuSettings>()->TrelloSettings;

	FString URL(TEXT("https://api.trello.com/1/cards/"));
	URL += AttachmentCardListID;
	URL += TEXT("/attachments?");
	URL += TEXT("&key=");
	URL += TrelloSettings.AccessKey;
	URL += TEXT("&token=");
	URL += TrelloSettings.AccessToken;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterTrello::OnResponseReceivedUploaded);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));

	const FString BoundaryKey = MakeBoundaryString();
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *BoundaryKey));

	TArray<uint8> ContentData;
	
	/* 画像データ */
	const FString DataName = FString::Printf(TEXT("name=\"file\";  filename=\"%s.jpg\""),*ScreenshotCapturedDateTime.ToString());
	AddContent(BoundaryKey,DataName, TEXT("image/jpeg"), ScreenshotImageData, ContentData);
	AddEndContentString(BoundaryKey, ContentData);

	Request->SetContent(ContentData);
	Request->ProcessRequest();
}

void AGDMRequesterTrello::RequestUploadLog()
{
	const FGDMTrelloSettings& TrelloSettings = GetDefault<UGameDebugMenuSettings>()->TrelloSettings;

	FString URL(TEXT("https://api.trello.com/1/cards/"));
	URL += AttachmentCardListID;
	URL += TEXT("/attachments?");
	URL += TEXT("&key=");
	URL += TrelloSettings.AccessKey;
	URL += TEXT("&token=");
	URL += TrelloSettings.AccessToken;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterTrello::OnResponseReceivedLog);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));

	const FString BoundaryKey = MakeBoundaryString();
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *BoundaryKey));

	TArray<uint8> ContentData;

	/* ゲーム内ログ */
	FString GameLog;
	GetOwnerDebugMenuManager()->GetOutputLogString(GameLog, LineBreak);
	const FString DataName = FString::Printf(TEXT("name=\"file\";  filename=\"%s.txt\""), *UKismetSystemLibrary::GetGameName());
	AddContentString(BoundaryKey, DataName, TEXT("text/plain; charset=UTF-8"), GameLog, ContentData);

	AddEndContentString(BoundaryKey, ContentData);

	Request->SetContent(ContentData);
	Request->ProcessRequest();
}

void AGDMRequesterTrello::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceived Success [%s] ResponseCode[%d]"), *ContentAsString, Response->GetResponseCode());

		const auto JsonReader = TJsonReaderFactory<TCHAR>::Create(ContentAsString);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			AttachmentCardListID = JsonObject->GetStringField(TEXT("id"));

			if(!AttachmentCardListID.IsEmpty())
			{
				if( bSendScreenshotCapture && (ScreenshotImageData.Num() > 0) )
				{
					/* 画像データを送信 */
					RequestUploadScreenshotCapture();
				}
				else if(bSendLogs)
				{
					/* ログを送信 */
					RequestUploadLog();
				}
				else
				{
					SuccessRequest();
				}
			}
			else
			{
				SuccessRequest();
			}
		}
		else
		{
			UE_LOG(LogGDM, Warning, TEXT("OnResponseReceived: Deserialize error"));
			FailedRequest();
		}
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceived: failed to connect"));
		FailedRequest();
	}
}

void AGDMRequesterTrello::OnResponseReceivedUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceivedUploaded: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());

		const auto JsonReader = TJsonReaderFactory<TCHAR>::Create(ContentAsString);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		if(FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			if(bSendLogs)
			{
				/* ログデータを送信 */
				RequestUploadLog();
			}
			else
			{
				SuccessRequest();
			}
		}
		else
		{
			UE_LOG(LogGDM, Warning, TEXT("OnResponseReceivedUploaded: Deserialize error"));
			FailedRequest();
		}
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceivedUploaded: failed to connect"));
		FailedRequest();
	}
}

void AGDMRequesterTrello::OnResponseReceivedLog(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceivedLog: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());
		SuccessRequest();
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceivedLog: failed to connect"));
		FailedRequest();
	}
}
