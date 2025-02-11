/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GDMPropertyJsonSystemComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SaveGame.h"
#include "GDMSaveSystemComponent.generated.h"

class UGDMSaveGame;

/**
 * DebugMenuのセーブ/ロード機能を扱うコンポーネント
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class GAMEDEBUGMENU_API UGDMSaveSystemComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	int32 UserIndex;

	UPROPERTY()
	TObjectPtr<UGDMSaveGame> SaveGame;
	
public:
	UGDMSaveSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	/**
	 * DebugMenuの状態を保存する 
	 */
	UFUNCTION(BlueprintCallable)
	virtual void SaveDebugMenuFile();
	
	/**
	 * DebugMenuの状態を読み込む
	 */
	UFUNCTION(BlueprintCallable)
	virtual void LoadDebugMenuFile();
	
	/**
	 * DebugMenuのセーブデータを削除する
	 */
	UFUNCTION(BlueprintCallable)
	virtual void DeleteDebugMenuFile();
	
protected:
	UGDMPropertyJsonSystemComponent* GetPropertyJsonSystemComponent() const;
	virtual bool SaveFile(const FString& ContentString);
	virtual bool LoadFile(FString& OutLoadedContentString);
	virtual bool DeleteFile();
	bool CanUseSaveGame();
};

UCLASS()
class UGDMSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Json;
};