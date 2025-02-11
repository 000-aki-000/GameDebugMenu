/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Reports/GDMRequesterRedmine.h"
#include "HttpModule.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "GameDebugMenuSettings.h"
#include <Kismet/KismetSystemLibrary.h>
#include "GameDebugMenuManager.h"


void AGDMRequesterRedmine::StartRequest()
{
	bWasRequestSuccessful = false;
	TokenScreenshotCapture.Reset();
	TokenLog.Reset();

	if(bSendScreenshotCapture && (ScreenshotImageData.Num() > 0))
	{
		RequestUploadScreenshotCapture();
	}
	else if(bSendLogs)
	{
		RequestUploadLog();
	}
	else
	{
		RequestIssues();
	}
}

void AGDMRequesterRedmine::RequestUploadScreenshotCapture()
{
	FString URL(TEXT("http://"));
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.HostName;
	URL += TEXT("/redmine/uploads.json?key=");
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.AccessKey;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterRedmine::OnResponseReceivedUploaded);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
	Request->SetContent(ScreenshotImageData);
	Request->ProcessRequest();
}

void AGDMRequesterRedmine::RequestUploadLog()
{
	/* ゲーム内ログ */
	FString GameLog;
	GetOwnerDebugMenuManager()->GetOutputLogString(GameLog, LineBreak);

	FString URL(TEXT("http://"));
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.HostName;
	URL += TEXT("/redmine/uploads.json?key=");
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.AccessKey;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterRedmine::OnResponseReceivedLog);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
	Request->SetContentAsString(GameLog);
	Request->ProcessRequest();
}

void AGDMRequesterRedmine::RequestIssues()
{
	FString URL(TEXT("http://"));
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.HostName;
	URL += TEXT("/redmine/issues.json?key=");
	URL += GetDefault<UGameDebugMenuSettings>()->RedmineSettings.AccessKey;

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterRedmine::OnResponseReceived);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	const TSharedRef<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject());
	const TSharedRef<FJsonObject> JsonIssueObject = MakeShareable(new FJsonObject());
	JsonRootObject->SetObjectField(TEXT("issue"), JsonIssueObject);
	JsonIssueObject->SetStringField(TEXT("project_id"), FString::FromInt(GetDefault<UGameDebugMenuSettings>()->RedmineSettings.ProjectId));
	JsonIssueObject->SetStringField(TEXT("tracker_id"), FString::FromInt(IssueCategoryIndex + 1));
	JsonIssueObject->SetStringField(TEXT("priority_id"), FString::FromInt(PriorityIndex + 1));
	JsonIssueObject->SetStringField(TEXT("subject"), GetSubject());
	JsonIssueObject->SetStringField(TEXT("description"), GetDescription());

	TArray< TSharedPtr< FJsonValue > > UploadObjects;
	const TSharedRef<FJsonObject> JsonUploadObjectCapture = MakeShareable(new FJsonObject());
	const TSharedRef<FJsonObject> JsonUploadObjectLog     = MakeShareable(new FJsonObject());

	bool bUploads = false;
	if (!TokenScreenshotCapture.IsEmpty())
	{
		bUploads = true;
		UploadObjects.Add(MakeShareable(new FJsonValueObject(JsonUploadObjectCapture)));
		JsonUploadObjectCapture->SetStringField(TEXT("token"), TokenScreenshotCapture);
		JsonUploadObjectCapture->SetStringField(TEXT("filename"), FString::Printf(TEXT("%s.jpg"), *ScreenshotCapturedDateTime.ToString()));
		JsonUploadObjectCapture->SetStringField(TEXT("content_type"), TEXT("image/jpg"));
	}

 	if(!TokenLog.IsEmpty())
 	{
		bUploads = true;
 		UploadObjects.Add(MakeShareable(new FJsonValueObject(JsonUploadObjectLog)));
		JsonUploadObjectLog->SetStringField(TEXT("token"), TokenLog);
		JsonUploadObjectLog->SetStringField(TEXT("filename"), FString::Printf(TEXT("%s.txt"), *UKismetSystemLibrary::GetGameName()));
		JsonUploadObjectLog->SetStringField(TEXT("content_type"), TEXT("text/plain"));
 	}

	if(bUploads)
	{
		JsonIssueObject->SetArrayField("uploads", UploadObjects);
	}

	FString Content;
	const auto Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonRootObject, Writer);
	Request->SetContentAsString(Content);

	Request->ProcessRequest();
}

void AGDMRequesterRedmine::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful != false)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceived: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());
		SuccessRequest();
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceived: failed to connect"));
		FailedRequest();
	}
}

void AGDMRequesterRedmine::OnResponseReceivedUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceivedUploaded: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());

		const auto JsonReader = TJsonReaderFactory<TCHAR>::Create(ContentAsString);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			const auto UploadObj = JsonObject->GetObjectField(TEXT("upload"));
			TokenScreenshotCapture = UploadObj->GetStringField(TEXT("token"));
			if(bSendLogs)
			{
				RequestUploadLog();
			}
			else
			{
				RequestIssues();
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

void AGDMRequesterRedmine::OnResponseReceivedLog(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceivedLog: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());

		const auto JsonReader = TJsonReaderFactory<TCHAR>::Create(ContentAsString);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		if(FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			const auto UploadObj = JsonObject->GetObjectField(TEXT("upload"));
			TokenLog = UploadObj->GetStringField(TEXT("token"));
			RequestIssues();
		}
		else
		{
			UE_LOG(LogGDM, Warning, TEXT("OnResponseReceivedLog: Deserialize error"));
			FailedRequest();
		}
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceivedLog: failed to connect"));
		FailedRequest();
	}
}
