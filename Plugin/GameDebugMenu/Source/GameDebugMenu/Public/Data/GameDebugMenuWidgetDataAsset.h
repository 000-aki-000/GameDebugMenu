/**
* Copyright (c) 2023 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameDebugMenuWidgetDataAsset.generated.h"

class UGameDebugMenuRootWidget;
class UGameDebugMenuWidget;

/**
 * 
 */
UCLASS()
class GAMEDEBUGMENU_API UGameDebugMenuWidgetDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** メインとなるデバックメニューのWidgetクラス */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GDM")
	TSubclassOf<UGameDebugMenuRootWidget> DebugMenuRootWidgetClass;

	/** 各メニューのWidget郡(Key=メニュー識別名, Value=メニュークラス) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GDM")
	TMap<FString, TSubclassOf<UGameDebugMenuWidget> > DebugMenuClasses;

	/** 各メニューのUI登録順（メニュー識別名を指定する） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "GDM")
	TArray<FString> DebugMenuRegistrationOrder;

	/** RootのDebugWidgetのZオーダー値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GDM")
	int32 WidgetZOrder;

public:
	UGameDebugMenuWidgetDataAsset();
};
