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
#include "GDMSaveSystemComponent.generated.h"

/**
 * DebugMenuのセーブ/ロード機能を扱うコンポーネント
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMSaveSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGDMSaveSystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	/**
	 * DebugMenuの状態を保存する 
	 */
	UFUNCTION(BlueprintCallable)
	virtual void SaveDebugMenuFile() const;
	
	/**
	 * DebugMenuの状態を読み込む
	 */
	UFUNCTION(BlueprintCallable)
	virtual void LoadDebugMenuFile() const;

protected:
	UGDMPropertyJsonSystemComponent* GetPropertyJsonSystemComponent() const;

};
