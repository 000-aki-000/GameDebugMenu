/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMInputTriggerPulseWithDelay.h"

#include "EnhancedPlayerInput.h"

FString UGDMInputTriggerPulseWithDelay::GetDebugState() const
{
	const float TimeSinceFirst = FMath::Max(0.f, HeldDuration - InitialDelay);
	const float TimeToNext = (TriggerCount == 0) ? InitialDelay - HeldDuration : RepeatInterval - FMath::Fmod(TimeSinceFirst, RepeatInterval);
	
	return FString::Printf(TEXT("Held:%.2f Triggers:%d TimeToNext:%.2f"), HeldDuration, TriggerCount, TimeToNext);
}

ETriggerState UGDMInputTriggerPulseWithDelay::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue ModifiedValue, float DeltaTime)
{
	ETriggerState State = ETriggerState::None;

	// Transition to Ongoing on actuation. Update the held duration.
	if (IsActuated(ModifiedValue))
	{
		State = ETriggerState::Ongoing;
		HeldDuration = CalculateHeldDuration(PlayerInput, DeltaTime);
	}
	else
	{
		// Reset duration
		HeldDuration = 0.0f;
	}
	
	if (State == ETriggerState::None)
	{
		bWasPressed = false;
		bTriggeredOnStart = false;
		TriggerCount = 0;
		return ETriggerState::None;
	}

	/* 初回押下処理 */
	if (!bWasPressed)
	{
		bWasPressed = true;

		if (bTriggerOnStart)
		{
			bTriggeredOnStart = true;
			++TriggerCount;
			return ETriggerState::Triggered;
		}
	}

	/* トリガー制限チェック */
	if (TriggerLimit > 0 && TriggerCount >= TriggerLimit)
	{
		return ETriggerState::None;
	}

	/* 初回トリガー（押下後 InitialDelay 経過） */
	if (!bTriggeredOnStart && TriggerCount == 0 && HeldDuration >= InitialDelay)
	{
		++TriggerCount;
		return ETriggerState::Triggered;
	}
	
	/* 2回目以降（RepeatInterval ごと） */
	if (TriggerCount > 0)
	{
		const float TimeSinceFirst = HeldDuration - InitialDelay;
		const int32 ExpectedCount = FMath::FloorToInt(TimeSinceFirst / RepeatInterval) + 1;
	
		if (ExpectedCount > TriggerCount)
		{
			++TriggerCount;
			return ETriggerState::Triggered;
		}
	}

	return ETriggerState::Ongoing;
}

float UGDMInputTriggerPulseWithDelay::CalculateHeldDuration(const UEnhancedPlayerInput* const PlayerInput, const float DeltaTime) const
{
	// We may not have a PlayerInput object during automation tests, so default to 1.0f if we don't have one.
	// This will mean that TimeDilation has no effect.
	const float TimeDilation = PlayerInput ? PlayerInput->GetEffectiveTimeDilation() : 1.0f;
	
	// Calculates the new held duration, applying time dilation if desired
	return HeldDuration + (!bAffectedByTimeDilation ? DeltaTime : DeltaTime * TimeDilation);
}
