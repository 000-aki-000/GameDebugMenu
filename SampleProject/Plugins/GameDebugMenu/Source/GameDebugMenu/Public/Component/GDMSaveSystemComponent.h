/**
* Copyright (c) 2024 akihiko moroi
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
UCLASS()
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

protected:
	UGDMPropertyJsonSystemComponent* GetPropertyJsonSystemComponent() const;
	bool SaveFile(const FString& ContentString);
	bool LoadFile(FString& OutLoadedContentString);
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