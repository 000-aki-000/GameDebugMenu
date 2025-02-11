/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Reports/GDMRequesterJira.h"
#include <HttpModule.h>
#include <Misc/Base64.h>
#include <GameDebugMenuManager.h>
#include <Kismet/KismetSystemLibrary.h>
#include <GameDebugMenuSettings.h>

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void AGDMRequesterJira::StartRequest()
{
	bWasRequestSuccessful = false;

	const FGDMJiraSettings& JiraSettings = GetDefault<UGameDebugMenuSettings>()->JiraSettings;
	const FString BasicAuthData          = TEXT("Basic ") + FBase64::Encode(JiraSettings.UserName + TEXT(":") + JiraSettings.AccessKey);
	const FString URL                    = TEXT("https://") + JiraSettings.HostName + TEXT("/rest/api/3/issue");

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
 	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterJira::OnResponseReceived);
 	Request->SetURL(URL);
 	Request->SetVerb(TEXT("POST"));
 	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), BasicAuthData);

	/*こんな形式のJsonを作ってる
	{
		"fields": {
			"project": {
				"key": "AAA"
			},
			"priority":{
				"name": "BBB"
			},
			"summary": "CCC",
			"issuetype": {
				"name": "DDD"
			},
			"description": {
				"type": "doc",
				"version": 1,
				"content": [
					{
						"type": "paragraph",
						"content": [
							{
								"text": "EEE",
								"type": "text"
							}
						]
					}
				]
			},
		}
	}
	*/
 	TSharedRef<FJsonObject> JsonRoot   = MakeShareable(new FJsonObject());
	const TSharedRef<FJsonObject> JsonFields = MakeShareable(new FJsonObject());

	JsonRoot->SetObjectField(TEXT("fields"), JsonFields);
	{
		const TSharedRef<FJsonObject> JsonProject = MakeShareable(new FJsonObject());
		JsonFields->SetObjectField(TEXT("project"), JsonProject);
		{
			JsonProject->SetStringField(TEXT("key"), JiraSettings.ProjectKeyName);
		}

		if(JiraSettings.PriorityNameList.IsValidIndex(PriorityIndex))
		{
			const TSharedRef<FJsonObject> JsonPriority = MakeShareable(new FJsonObject());
			JsonFields->SetObjectField(TEXT("priority"), JsonPriority);
			{
				const FString PriorityStr = JiraSettings.PriorityNameList[PriorityIndex].ToString();
				JsonPriority->SetStringField(TEXT("name"), PriorityStr);
			}
		}

		const FString AccountId = JiraSettings.GetAssigneeAccountIdByListIndex(AssigneeIndex);
		if(!AccountId.IsEmpty())
		{
			const TSharedRef<FJsonObject> JsonAssignee = MakeShareable(new FJsonObject());
			JsonFields->SetObjectField(TEXT("assignee"), JsonAssignee);
			{
				JsonAssignee->SetStringField(TEXT("accountId"), AccountId);
			}
		}

		JsonFields->SetStringField(TEXT("summary"), GetSubject());

		if(JiraSettings.IssueTypeList.IsValidIndex(IssueCategoryIndex))
		{
			const TSharedRef<FJsonObject> JsonIssueType = MakeShareable(new FJsonObject());
			JsonFields->SetObjectField(TEXT("issuetype"), JsonIssueType);
			{
				const FString TypeStr = JiraSettings.IssueTypeList[IssueCategoryIndex].ToString();
				JsonIssueType->SetStringField(TEXT("name"), TypeStr);
			}
		}

		if( JiraSettings.LabelNameList.IsValidIndex(0))
		{
			TArray< TSharedPtr<FJsonValue> > JsonLabelArray;
			for( const auto& Label : JiraSettings.LabelNameList )
			{
				JsonLabelArray.Add(MakeShareable(new FJsonValueString(Label)));
			}
			JsonFields->SetArrayField(TEXT("labels"),JsonLabelArray);
		}

		const TSharedRef<FJsonObject> JsonDescription = MakeShareable(new FJsonObject());
		JsonFields->SetObjectField(TEXT("description"), JsonDescription);
		{
			JsonDescription->SetStringField(TEXT("type"), TEXT("doc"));
			JsonDescription->SetNumberField(TEXT("version"), 1);

			const TSharedRef<FJsonObject> JsonContent = MakeShareable(new FJsonObject());
			TArray< TSharedPtr< FJsonValue > > Contents;
			Contents.Reserve(1);
			Contents.Emplace(MakeShareable(new FJsonValueObject(JsonContent)));

			JsonDescription->SetArrayField(TEXT("content"), Contents);
			{
				JsonContent->SetStringField(TEXT("type"), TEXT("paragraph"));

				const TSharedRef<FJsonObject> JsonContent_2 = MakeShareable(new FJsonObject());
				TArray< TSharedPtr< FJsonValue > > Contents_2;
				Contents_2.Reserve(1);
				Contents_2.Emplace(MakeShareable(new FJsonValueObject(JsonContent_2)));

				JsonContent->SetArrayField(TEXT("content"), Contents_2);
				{
					JsonContent_2->SetStringField(TEXT("text"), GetDescription());
					JsonContent_2->SetStringField(TEXT("type"), TEXT("text"));
				}
			}
		}
	}

 	FString Content;
	const auto Writer = TJsonWriterFactory<>::Create(&Content);
 	FJsonSerializer::Serialize(JsonRoot, Writer);
 	Request->SetContentAsString(Content);
 	Request->ProcessRequest();
}

void AGDMRequesterJira::RequestUploadContent(const FString& IssueKey)
{
	const FGDMJiraSettings& JiraSettings = GetDefault<UGameDebugMenuSettings>()->JiraSettings;
	const FString BasicAuthData          = TEXT("Basic ") + FBase64::Encode(JiraSettings.UserName + TEXT(":") + JiraSettings.AccessKey);
	const FString URL                    = TEXT("https://") + JiraSettings.HostName + TEXT("/rest/api/3/issue/") + IssueKey + TEXT("/attachments");
	const FString BoundaryKey			 = MakeBoundaryString();

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGDMRequesterJira::OnResponseReceivedUploaded);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("X-Atlassian-Token"), TEXT("no-check"));
 	Request->SetHeader(TEXT("Authorization"), BasicAuthData);
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *BoundaryKey));

	TArray<uint8> ContentData;

	/* 画像データ */
	if(bSendScreenshotCapture && (ScreenshotImageData.Num() > 0))
	{
		const FString DataName = FString::Printf(TEXT("name=\"file\";  filename=\"%s.jpg\""), *ScreenshotCapturedDateTime.ToString());
		AddContent(BoundaryKey, DataName, TEXT("image/jpeg"), ScreenshotImageData, ContentData);
	}

	/* ゲーム内ログ */
	if(bSendLogs)
	{
		FString GameLog;
		GetOwnerDebugMenuManager()->GetOutputLogString(GameLog, LineBreak);
		const FString DataName = FString::Printf(TEXT("name=\"file\";  filename=\"%s.txt\""), *UKismetSystemLibrary::GetGameName());
		AddContentString(BoundaryKey, DataName, TEXT("text/plain; charset=UTF-8"), GameLog, ContentData);
	}

	AddEndContentString(BoundaryKey, ContentData);

	Request->SetContent(ContentData);
	Request->ProcessRequest();
}

void AGDMRequesterJira::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful != false)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceived: Success %s: ResponseCode %d"), *ContentAsString, Response->GetResponseCode());

		const auto JsonReader = TJsonReaderFactory<TCHAR>::Create(ContentAsString);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		if(FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			const FString IssueKey = JsonObject->GetStringField(TEXT("key"));
			if( !IssueKey.IsEmpty() && (bSendScreenshotCapture || bSendLogs) )
			{
				RequestUploadContent(IssueKey);
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

void AGDMRequesterJira::OnResponseReceivedUploaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		const FString ContentAsString = Response->GetContentAsString();
		UE_LOG(LogGDM, Log, TEXT("OnResponseReceivedUploaded: Success %s ResponseCode %d"), *ContentAsString, Response->GetResponseCode());
		SuccessRequest();
	}
	else
	{
		UE_LOG(LogGDM, Error, TEXT("OnResponseReceivedUploaded: failed to connect"));
		FailedRequest();
	}
}
