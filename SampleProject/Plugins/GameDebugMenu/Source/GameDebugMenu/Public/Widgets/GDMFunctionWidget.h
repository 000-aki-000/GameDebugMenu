/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuWidget.h"
#include "GameDebugMenuTypes.h"
#include "GDMFunctionWidget.generated.h"

/**
 * 
 */
UCLASS()
class UGDMFunctionWidget : public UGameDebugMenuWidget
{
	GENERATED_BODY()

public:
	/* 関数所持オブジェクト */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Functions")
	UObject* TargetObject;

	/* 関数名 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Functions")
	FName FunctionName;

	UPROPERTY(BlueprintReadWrite, Category = "GDM|Functions")
	FString FunctionSaveKey;
	
public:
	UFUNCTION(BlueprintCallable)
	bool TryCallObjectFunction(FName EventName);

protected:
	bool GDMProcessEvent(FName EventName, void* Parms);
};
