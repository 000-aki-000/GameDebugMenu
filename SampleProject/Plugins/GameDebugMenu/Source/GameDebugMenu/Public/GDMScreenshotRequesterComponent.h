/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMListenerComponent.h"
#include "GDMScreenshotRequesterComponent.generated.h"

class AGameDebugMenuManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGDMGameDebugMenuScreenshotDelegate);

/**
* DebugMenuでのスクショ処理
*/
UCLASS()
class GAMEDEBUGMENU_API UGDMScreenshotRequesterComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	FDelegateHandle OnScreenshotCapturedHandle;
	FDelegateHandle OnScreenshotRequestProcessedHandle;
	bool bRequestProcessed;

public:	
	UGDMScreenshotRequesterComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void RequestScreenshot();
	virtual bool IsRequestProcessed();

protected:
	AGameDebugMenuManager* GetOwnerGameDebugMenuManager() const;
	virtual void OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap);
	virtual void OnScreenshotRequestProcessed();
	virtual void ResetHandle();
};
