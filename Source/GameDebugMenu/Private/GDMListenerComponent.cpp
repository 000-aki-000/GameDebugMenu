/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GDMListenerComponent.h"
#include <Engine/World.h>

TMap<UWorld*, TArray<UGDMListenerComponent*>> UGDMListenerComponent::CacheListenerComponents;

UGDMListenerComponent::UGDMListenerComponent()
{
	PrimaryComponentTick.bCanEverTick          = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bNeverNeedsRenderUpdate = true;
	bWantsInitializeComponent = true;
}

void UGDMListenerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	PushListenerComponent(GetWorld(),this);
}

void UGDMListenerComponent::UninitializeComponent()
{
	PopListenerComponent(GetWorld(),this);
	Super::UninitializeComponent();
}

void UGDMListenerComponent::AllUnbindDispatchers()
{
	OnShowDispatcher.Clear();
	OnHideDispatcher.Clear();
	OnExecuteConsoleCommandDispatcher.Clear();
	OnExecuteProcessEventDispatcher.Clear();
	OnChangePropertyBoolDispatcher.Clear();
	OnChangePropertyIntDispatcher.Clear();
	OnChangePropertyFloatDispatcher.Clear();
	OnChangePropertyByteDispatcher.Clear();
	OnChangePropertyStringDispatcher.Clear();
	OnRegisterInputSystemDispatcher.Clear();
	OnUnregisterInputSystemDispatcher.Clear();
	OnChangeActiveInputObjectDispatcher.Clear();
}

int32 UGDMListenerComponent::PushListenerComponent(UWorld* TargetWorld, UGDMListenerComponent* Listener)
{
	TArray<UGDMListenerComponent*>& ListenerComponents = CacheListenerComponents.FindOrAdd(TargetWorld);
	return ListenerComponents.AddUnique(Listener);
}

int32 UGDMListenerComponent::PopListenerComponent(UWorld* TargetWorld, UGDMListenerComponent* Listener)
{
	TArray<UGDMListenerComponent*>& ListenerComponents = CacheListenerComponents.FindOrAdd(TargetWorld);
	return ListenerComponents.Remove(Listener);
}

void UGDMListenerComponent::GetAllListenerComponents(UWorld* TargetWorld, TArray<UGDMListenerComponent*>& OutListenerComponents)
{
	TArray<UGDMListenerComponent*>& ListenerComponents = CacheListenerComponents.FindOrAdd(TargetWorld);
	OutListenerComponents.Reserve(ListenerComponents.Num());

	for( int32 Index = ListenerComponents.Num() - 1; Index >= 0; --Index )
	{
		if( IsValid(ListenerComponents[Index]) )
		{
			OutListenerComponents.Add(ListenerComponents[Index]);
		}
		else
		{
			ListenerComponents.RemoveAt(Index);
		}
	}
}
