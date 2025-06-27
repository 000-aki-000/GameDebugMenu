/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "GDMInputTriggerPulseWithDelay.generated.h"

/**
 * 最初と２回目以降のトリガー時間を別に指定できるInputTrigger
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "GDMPulseWithDelay"))
class GAMEDEBUGMENU_API UGDMInputTriggerPulseWithDelay : public UInputTrigger/* UInputTriggerTimedBaseのENHANCEDINPUT_APIがなぜかUE5.6で削除された.... */
{
	GENERATED_BODY()

public:
	/** 押した瞬間にも発火させるか */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings")
	bool bTriggerOnStart = true;
	
	/** 最初の1回目を発火するまでの遅延（秒） */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings", meta = (ClampMin = "0"))
	float InitialDelay = 0.5f;

	/** 2回目以降のトリガー間隔（秒） */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings", meta = (ClampMin = "0"))
	float RepeatInterval = 0.1f;

	/** 入力が保持されている間の最大トリガー回数（0 = 無制限） */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings", meta = (ClampMin = "0"))
	int32 TriggerLimit = 0;

	// How long have we been actuating this trigger?
	UPROPERTY(BlueprintReadWrite, Category = "Trigger Settings")
	float HeldDuration = 0.0f;

	/**
	 * Should global time dilation be applied to the held duration?
	 * Default is set to false.
	 * 
	 * If this is set to true, then the owning Player Controller's actor time dilation
	 * will be used when calculating the HeldDuration.
	 * 
	 * @see UInputTriggerTimedBase::CalculateHeldDuration
	 * @see AWorldSettings::GetEffectiveTimeDilation
	 */
	UPROPERTY(EditAnywhere, Config, BlueprintReadWrite, Category = "Trigger Settings")
	bool bAffectedByTimeDilation = false;
	
private:
	bool bWasPressed = false;
	bool bTriggeredOnStart = false;
	int32 TriggerCount = 0;
	
public:
	virtual FString GetDebugState() const override;
	virtual ETriggerEventsSupported GetSupportedTriggerEvents() const override { return ETriggerEventsSupported::Ongoing; }
	virtual ETriggerState UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime) override;
	float CalculateHeldDuration(const UEnhancedPlayerInput* PlayerInput, float DeltaTime) const;
};
