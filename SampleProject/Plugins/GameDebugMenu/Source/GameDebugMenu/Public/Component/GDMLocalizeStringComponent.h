/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuTypes.h"
#include "Components/ActorComponent.h"
#include "GDMLocalizeStringComponent.generated.h"

class UGDMPropertyJsonSystemComponent;

/**
 * DebugMenuの文字列管理
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class GAMEDEBUGMENU_API UGDMLocalizeStringComponent : public UActorComponent
{
	GENERATED_BODY()

	/** 読み込んだ文字のキーと現在の言語にあった文字列 */
	UPROPERTY(Transient)
	TMap<FString, FString> CachedDebugMenuStrings;

	/** True: デバックメニュー用の StringKey を指定してる箇所をそのまま表示する */
	UPROPERTY(Transient)
	bool bCurrentDebugMenuDirectStringKey;

	/** 現在のデバックメニューの言語 */
	UPROPERTY(Transient)
	FName CurrentLanguage;
	
public:
	UGDMLocalizeStringComponent();

public:
	/**
	 * Json情報を取得しセットする
	 */
	void SetJsonSystemComponentValue(UGDMPropertyJsonSystemComponent* PropertyJsonSystemComponent);

	/**
	 * Json情報を取得しセットする
	 */
	void SetToJsonSystemComponent(UGDMPropertyJsonSystemComponent* PropertyJsonSystemComponent, const FString& Language);

	/**
	 * DebugMenu用のローカライズされた文字列を返す
	 */
	bool GetString(const FString& StringKey, FString& OutString);
	
	void SyncLoadDebugMenuStringTables();
	
	const FName& GetCurrentDebugMenuLanguage() const;
};

/************************************************************************************************************************************/

inline bool UGDMLocalizeStringComponent::GetString(const FString& StringKey, FString& OutString)
{
	if( FString* SourceString = CachedDebugMenuStrings.Find(StringKey) )
	{
		if( bCurrentDebugMenuDirectStringKey )
		{
			/* 取得できたキーをそのまま戻す */
			OutString = StringKey;
		}
		else
		{
			OutString = *SourceString;
		}
	}
	else
	{
		OutString = StringKey;
		UE_LOG(LogGDM, Verbose, TEXT("GetDebugMenuString: Not found StringKey %s"), *StringKey);
		return false;
	}

	return true;
}

inline const FName& UGDMLocalizeStringComponent::GetCurrentDebugMenuLanguage() const
{
	return CurrentLanguage;
}
