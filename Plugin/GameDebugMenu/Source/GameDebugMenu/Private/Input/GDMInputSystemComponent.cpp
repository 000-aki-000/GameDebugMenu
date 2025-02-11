/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMInputSystemComponent.h"
#include <Engine/World.h>

#include "GameDebugMenuFunctions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/WorldSettings.h"
#include "Input/GDMInputEventFunctions.h"
#include "Input/GDMInputInterface.h"
#include "GameDebugMenuSettings.h"
#include "GameDebugMenuManager.h"
#include "Engine/DebugCameraController.h"
#include "GameFramework/CheatManager.h"
#include "Input/GDMDebugCameraInput.h"

/************************************************************************/
/* RepeatButtonInfo											   */
/************************************************************************/

UGDMInputSystemComponent::RepeatButtonInfo::RepeatButtonInfo()
	: Instance(nullptr)
	, CallFunction(nullptr)
	, bRun(false)
	, ElapsedTime(0.0f)
	, EventName(NAME_None)
{
}

UGDMInputSystemComponent::RepeatButtonInfo::RepeatButtonInfo(UGDMInputSystemComponent* Ins)
	: Instance(Ins)
	, CallFunction(nullptr)
	, bRun(false)
	, ElapsedTime(0.0f)
	, EventName(NAME_None)
{
}

void UGDMInputSystemComponent::RepeatButtonInfo::SetFunction(FUNCTION Function)
{
	CallFunction = Function;
}

void UGDMInputSystemComponent::RepeatButtonInfo::Tick(float DeltaTime)
{
	if(CallFunction == nullptr)
	{
		return;
	}

	if(!bRun)
	{
		return;
	}

	ElapsedTime -= DeltaTime;

	if(ElapsedTime <= 0.0f)
	{
		ElapsedTime = GetDefault<UGameDebugMenuSettings>()->ButtonRepeatInterval;

		/* イベント呼び出し */
		(Instance->*CallFunction)(EventName);
	}
}

void UGDMInputSystemComponent::RepeatButtonInfo::Start()
{
	if(bRun)
	{
		return;
	}

	bRun = true;
	ElapsedTime = GetDefault<UGameDebugMenuSettings>()->ButtonRepeatDelay;
}

void UGDMInputSystemComponent::RepeatButtonInfo::Stop()
{
	if(!bRun)
	{
		return;
	}

	bRun = false;
	ElapsedTime = 0.0f;
}

/************************************************************************/
/* UGDMInputSystemComponent										*/
/************************************************************************/

UGDMInputSystemComponent::UGDMInputSystemComponent()
	: RepeatButtons()
	, RegisterInputObjects()
	, CurrentInputObject(nullptr)
	, CachePressedInputObjects()
	, bBindingsInputComponent(false)
	, IgnoreDebugMenuInput(0)
	, bActionMappingBindingsAdded(false)
	, DebugCameraInputClass(nullptr)
	, DebugCameraInput(nullptr)
	, bOutputDebugLog(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	bNeverNeedsRenderUpdate = true;
	DebugCameraInputClass = AGDMDebugCameraInput::StaticClass();
}

void UGDMInputSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!UKismetSystemLibrary::IsDedicatedServer(this))
	{
		InitializeInputMapping();
		SetupRepeatButtons();
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void UGDMInputSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(IsValid(DebugCameraInput))
	{
		DebugCameraInput->K2_DestroyActor();
		DebugCameraInput = nullptr;
	}
}

void UGDMInputSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	/* Slomo など時間操作に影響受けないようにする */
	DeltaTime /= GetOwner()->GetWorldSettings()->GetEffectiveTimeDilation();

	TickRepeatButtons(DeltaTime);
	UpdateInputObject();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGDMInputSystemComponent::InitializeInputComponentBindings(UInputComponent* OwnerInputComponent)
{
	if(!IsValid(OwnerInputComponent))
	{
		return;
	}

	if(!bBindingsInputComponent)
	{
		bBindingsInputComponent = true;
		SetupInputComponentBindActions(OwnerInputComponent);
	}
}

bool UGDMInputSystemComponent::RegisterInputObject(UObject* TargetObject)
{
	if(!IsValid(TargetObject))
	{
		UE_LOG(LogGDM, Warning, TEXT("Failed register input [not found TargetObject]"));
		return false;
	}

	if(!TargetObject->Implements<UGDMInputInterface>())
	{
		UE_LOG(LogGDM, Warning, TEXT("Failed register input[Implement UGDMInputInterface]: %s "), *GetNameSafe(TargetObject));
		return false;
	}

	if(RegisterInputObjects.AddUnique(TargetObject) >= 0)
	{
		RegisterInputObjects.Sort([](const TWeakObjectPtr<UObject>& A, const TWeakObjectPtr<UObject>& B)
								  {
									  return IGDMInputInterface::Execute_GetInputPriorityGDM(A.Get()) <= IGDMInputInterface::Execute_GetInputPriorityGDM(B.Get());
								  });

		UE_LOG(LogGDM, Verbose, TEXT("Register input object: %s"), *GetNameSafe(TargetObject));
		IGDMInputInterface::Execute_OnRegisterGDMInputSystem(TargetObject);
		return true;
	}

	return false;
}

bool UGDMInputSystemComponent::UnregisterInputObject(UObject* TargetObject)
{
	if( !RegisterInputObjects.Contains(TargetObject) )
	{
		return false;
	}

	RegisterInputObjects.Remove(TargetObject);

	RegisterInputObjects.Sort([](const TWeakObjectPtr<UObject>& A, const TWeakObjectPtr<UObject>& B)
	{
		return IGDMInputInterface::Execute_GetInputPriorityGDM(A.Get()) <= IGDMInputInterface::Execute_GetInputPriorityGDM(B.Get());
	});

	UE_LOG(LogGDM, Verbose, TEXT("Unregister input object: %s"), *GetNameSafe(TargetObject));

	if( CurrentInputObject.Get() == TargetObject )
	{
		UE_LOG(LogGDM, Verbose, TEXT("Unregister Release event is called: %s"), *GetNameSafe(TargetObject));

		/* 入力対象のオブジェクトなので1度リリースを呼ぶ */
		CallReleasedButtons();
		CurrentInputObject = nullptr;
	}

	IGDMInputInterface::Execute_OnUnregisterGDMInputSystem(TargetObject);
	return true;
}

void UGDMInputSystemComponent::SetIgnoreInput(bool bNewInput)
{
	IgnoreDebugMenuInput = FMath::Max(IgnoreDebugMenuInput + (bNewInput ? +1 : -1), 0);
}

void UGDMInputSystemComponent::ResetIgnoreInput()
{
	IgnoreDebugMenuInput = 0;
}

bool UGDMInputSystemComponent::IsInputIgnored() const
{
	return (IgnoreDebugMenuInput > 0);
}

void UGDMInputSystemComponent::CreateDebugCameraInputClass()
{
	if(DebugCameraInputClass == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner                          = GetOwner();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	DebugCameraInput                         = GetWorld()->SpawnActor<AGDMDebugCameraInput>(DebugCameraInputClass, SpawnInfo);

	bool bExistDCC        = false;
	const APlayerController* PC = GetOwnerGameDebugMenuManager()->GetOwnerPlayerController();
	if(IsValid(PC->CheatManager))
	{
		if(ADebugCameraController* DCC = PC->CheatManager->DebugCameraControllerRef)
		{
			bExistDCC = true;
			DebugCameraInput->EnableInput(DCC);
		}
	}

	if(!bExistDCC)
	{
		/* ないので生成するのを待つ */
		DebugCameraInput->BindSpawnDebugCameraController();
	}
}

void UGDMInputSystemComponent::InitializeInputMapping()
{
	if(!bActionMappingBindingsAdded)
	{
		bActionMappingBindingsAdded = true;

		const auto& ActionMappingKeys = GetDefault<UGameDebugMenuSettings>()->ActionMappingKeys;
		for( const auto& Pair : ActionMappingKeys)
		{
			for( const auto& KeyEvent : Pair.Value.Keys)
			{
				UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping(Pair.Key, KeyEvent));
			}
		}

		const auto& AxisMappingKeys = GetDefault<UGameDebugMenuSettings>()->AxisMappingKeys;
		for( const auto& Pair : AxisMappingKeys)
		{
			const FGDMAxisMappingKey& Axis = Pair.Value;
			for(int32 Index = 0; Index < Axis.Keys.Num(); ++Index)
			{
				UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping(Pair.Key, Axis.Keys[Index], Axis.Scale[Index]));
			}
		}
	}
}

AGameDebugMenuManager* UGDMInputSystemComponent::GetOwnerGameDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

void UGDMInputSystemComponent::SetupInputComponentBindActions(UInputComponent* OwnerInputComponent)
{
	FInputActionBinding* InputActionBinding = nullptr;

	const auto& ActionMappingKeys = GetDefault<UGameDebugMenuSettings>()->ActionMappingKeys;
	for( const auto& Pair : ActionMappingKeys )
	{
		InputActionBinding = &OwnerInputComponent->BindAction<FGDMInputActionDelegate>(Pair.Key, EInputEvent::IE_Pressed, this, &UGDMInputSystemComponent::CallInputPressedInterfaceEvent, Pair.Key);
		InputActionBinding->bExecuteWhenPaused = true;

		InputActionBinding = &OwnerInputComponent->BindAction<FGDMInputActionDelegate>(Pair.Key, EInputEvent::IE_Released, this, &UGDMInputSystemComponent::CallInputReleasedInterfaceEvent, Pair.Key);
		InputActionBinding->bExecuteWhenPaused = true;
	}

	FInputAxisBinding* InputAxisBinding = nullptr;
	InputAxisBinding = &OwnerInputComponent->BindAxis(GDMInputEventNames::AxisMoveForward, this, &UGDMInputSystemComponent::OnAxisMoveForward);
	InputAxisBinding->bExecuteWhenPaused = true;
	InputAxisBinding = &OwnerInputComponent->BindAxis(GDMInputEventNames::AxisMoveRight, this, &UGDMInputSystemComponent::OnAxisMoveRight);
	InputAxisBinding->bExecuteWhenPaused = true;
	InputAxisBinding = &OwnerInputComponent->BindAxis(GDMInputEventNames::AxisLookRight, this, &UGDMInputSystemComponent::OnAxisLookRight);
	InputAxisBinding->bExecuteWhenPaused = true;
	InputAxisBinding = &OwnerInputComponent->BindAxis(GDMInputEventNames::AxisLookUp, this, &UGDMInputSystemComponent::OnAxisLookUp);
	InputAxisBinding->bExecuteWhenPaused = true;
}

UObject* UGDMInputSystemComponent::GetCurrentInputObject(bool bCheckEnableInput)
{
	if(IsInputIgnored())
	{
		return nullptr;
	}

	if(CurrentInputObject.IsValid())
	{
		if( bCheckEnableInput )
		{
			if( !IGDMInputInterface::Execute_IsEnableInputGDM(CurrentInputObject.Get()) )
			{
				return nullptr;
			}
		}

		return CurrentInputObject.Get();
	}
	return nullptr;
}

void UGDMInputSystemComponent::UpdateInputObject()
{
	UObject* NewInputObject = nullptr;

	for(int32 Index = RegisterInputObjects.Num() - 1; Index >= 0; --Index)
	{
		if(!RegisterInputObjects[Index].IsValid())
		{
			/* なくなっていたので削除 */
			UE_LOG(LogGDM, Error, TEXT("UpdateInputObject Not found InputObject: Index %d"), Index);
			RegisterInputObjects.RemoveAt(Index);
			continue;
		}

		if(!IGDMInputInterface::Execute_IsEnableInputGDM(RegisterInputObjects[Index].Get()))
		{
			/* 無効な場合 */
			if(IGDMInputInterface::Execute_IsModalModeGDM(RegisterInputObjects[Index].Get()))
			{
				/* モーダル化してるのでここで終了。低優先度オブジェクトが入力するには
				このオブジェクトがUnregisterGDMInputObjectされるかModalをFalseにするかのどっちか */
				return;
			}
			else
			{
				/* モーダルは無効になっているので次の優先度のオブジェクトに入力確認してく */
				continue;
			}
		}

		NewInputObject = RegisterInputObjects[Index].Get();
		break;
	}

	if(CurrentInputObject.Get() != NewInputObject)
	{
		UE_LOG(LogGDM, Verbose, TEXT("Change input object %s -> %s"), *GetNameSafe(CurrentInputObject.Get()) , *GetNameSafe(NewInputObject));

		/* 対象が変わったら１度リリースを呼び出す */
		CallReleasedButtons();

		UObject* OldObject = CurrentInputObject.Get();
		CurrentInputObject = NewInputObject;

		GetOwnerGameDebugMenuManager()->CallChangeActiveInputObjectDispatcher(NewInputObject,OldObject);
	}

	if( bOutputDebugLog )
	{
		for( int32 Index = 0; Index < RegisterInputObjects.Num(); ++Index )
		{
			UObject* Current    = RegisterInputObjects[Index].Get();
			const bool bIsEnableInput = IGDMInputInterface::Execute_IsEnableInputGDM(Current);
			const bool bIsModalMode   = IGDMInputInterface::Execute_IsModalModeGDM(Current);
			const int32 InputPriority = IGDMInputInterface::Execute_GetInputPriorityGDM(Current);

			FString LogString;
			if( CurrentInputObject.Get() != Current )
			{
				LogString = FString::Printf(TEXT("Input Objects | Priority=%d | Name=%s | IsEnableInput=%s | IsModalMode=%s")
											, InputPriority
											, *GetNameSafe(Current)
											, bIsEnableInput ? TEXT("true") : TEXT("false")
											, bIsModalMode ? TEXT("true") : TEXT("false")
											);
			}
			else
			{
				LogString = FString::Printf(TEXT("Input Objects | ** Current ** | Priority=%d | Name=%s | IsEnableInput=%s | IsModalMode=%s")
											, InputPriority
											, *GetNameSafe(Current)
											, bIsEnableInput ? TEXT("true") : TEXT("false")
											, bIsModalMode ? TEXT("true") : TEXT("false")
				);
			}

			UGameDebugMenuFunctions::PrintLogScreen(this, LogString, 0.0f/* Duration */);
		}
	}
}

void UGDMInputSystemComponent::CallInputPressedInterfaceEvent(FName EventName)
{
	if( EventName == GDMInputEventNames::MenuOpenAndClose )
	{
		if( IsInputIgnored() )
		{
			return;
		}

		GetOwnerGameDebugMenuManager()->HideDebugMenu();
		return;
	}

	const TWeakObjectPtr<UObject> Obj = GetCurrentInputObject();
	if(Obj.IsValid())
	{
		CachePressedInputObjects.Add(EventName, Obj);

		UE_LOG(LogGDM, Verbose, TEXT("Pressed input: obj: %s EventName: %s"), *GetNameSafe(Obj.Get()), *EventName.ToString());

		IGDMInputInterface::Execute_OnInputPressedGDM(Obj.Get(), EventName, false);

		if(RepeatButtonInfo* Repeat = RepeatButtons.Find(EventName))
		{
			Repeat->Start();
		}
	}
}

void UGDMInputSystemComponent::CallInputReleasedInterfaceEvent(FName EventName)
{
	if(const TWeakObjectPtr<UObject>* Obj = CachePressedInputObjects.Find(EventName))
	{
		CachePressedInputObjects.Remove(EventName);

		if(Obj->IsValid())
		{
			UE_LOG(LogGDM, Verbose, TEXT("Released input: obj: %s EventName: %s"), *GetNameSafe(Obj->Get()), *EventName.ToString());

			IGDMInputInterface::Execute_OnInputReleasedGDM(Obj->Get(), EventName);

			if(RepeatButtonInfo* Repeat = RepeatButtons.Find(EventName))
			{
				Repeat->Stop();
			}
		}
	}
}

void UGDMInputSystemComponent::CallReleasedButtons()
{
	const auto& ActionMappingKeys = GetDefault<UGameDebugMenuSettings>()->ActionMappingKeys;
	for( const auto& Pair : ActionMappingKeys )
	{
		CallInputReleasedInterfaceEvent(Pair.Key);
	}
}

void UGDMInputSystemComponent::OnAxisMoveForward(float Value)
{
	if(UObject* InputObj = GetCurrentInputObject())
	{
		IGDMInputInterface::Execute_OnInputAxisMoveForwardGDM(InputObj, Value);
	}
}

void UGDMInputSystemComponent::OnAxisMoveRight(float Value)
{
	if(UObject* InputObj = GetCurrentInputObject())
	{
		IGDMInputInterface::Execute_OnInputAxisMoveRightGDM(InputObj, Value);
	}
}

void UGDMInputSystemComponent::OnAxisLookRight(float Value)
{
	if(UObject* InputObj = GetCurrentInputObject())
	{
		IGDMInputInterface::Execute_OnInputAxisLookRightGDM(InputObj, Value);
	}
}

void UGDMInputSystemComponent::OnAxisLookUp(float Value)
{
	if(UObject* InputObj = GetCurrentInputObject())
	{
		IGDMInputInterface::Execute_OnInputAxisLookUpGDM(InputObj, Value);
	}
}

void UGDMInputSystemComponent::OnInputRepeat(const FName& EventName)
{
	if(const TWeakObjectPtr<UObject>* Obj = CachePressedInputObjects.Find(EventName))
	{
		if(!Obj->IsValid())
		{
			return;
		}

		if(!IGDMInputInterface::Execute_IsEnableInputGDM(Obj->Get()))
		{
			return;
		}

		UE_LOG(LogGDM, Verbose, TEXT("Repeat input: obj: %s EventName: %s"), *GetNameSafe(Obj->Get()), *EventName.ToString());

		IGDMInputInterface::Execute_OnInputPressedGDM(Obj->Get(), EventName, true);

		if(RepeatButtonInfo* Repeat = RepeatButtons.Find(EventName))
		{
			Repeat->Start();
		}
	}
}

void UGDMInputSystemComponent::SetupRepeatButtons()
{
	RepeatButtonInfo Repeat(this);
	Repeat.SetFunction(&UGDMInputSystemComponent::OnInputRepeat);

	const auto& ActionMappingKeys = GetDefault<UGameDebugMenuSettings>()->ActionMappingKeys;
	for( const auto& Pair : ActionMappingKeys )
	{
		if( Pair.Value.bRepeat )
		{
			Repeat.EventName = Pair.Key;
			RepeatButtons.Add(Pair.Key, Repeat);
		}
	}
}

void UGDMInputSystemComponent::TickRepeatButtons(const float DeltaTime)
{
	for(auto& Pair : RepeatButtons)
	{
		Pair.Value.Tick(DeltaTime);
	}
}
