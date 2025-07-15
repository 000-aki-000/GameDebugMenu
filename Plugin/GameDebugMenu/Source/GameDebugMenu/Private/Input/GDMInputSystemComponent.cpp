/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMInputSystemComponent.h"
#include <Engine/World.h>
#include "EnhancedInputSubsystems.h"
#include "Engine/DebugCameraController.h"
#include "GameFramework/CheatManager.h"

#include "GameDebugMenuFunctions.h"
#include "GameDebugMenuManager.h"
#include "Component/GDMLocalizeStringComponent.h"
#include "Input/GDMDebugCameraInput.h"
#include "Input/GDMEnhancedInputComponent.h"
#include "Widgets/GameDebugMenuRootWidget.h"

UGDMInputSystemComponent::UGDMInputSystemComponent()
	: IgnoreDebugMenuInput(0)
	, DebugCameraInput(nullptr)
	, RegisteredInputGroups()
	, ActiveInputStacks()
	, CurrentInputGroupName(NAME_None)
	, bMenuOpen(false)
	, RootWidgetInputComponent(nullptr)
	, AddInputMappingContextWhenCreateManager()
	, AddInputMappingContextWhenDebugMenuIsShow()
	, ActorSpawnedDelegateHandle()
	, DebugCameraController(nullptr)
	, bOutputDebugLog(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	bNeverNeedsRenderUpdate = true;
}

void UGDMInputSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bOutputDebugLog)
	{
		FString DebugText;
		
		const AGameDebugMenuManager* Manager = GetOwnerGameDebugMenuManager();
		FString GroupName;
		Manager->GetLocalizeStringComponent()->GetString(CurrentInputGroupName.ToString(), GroupName);
		
		DebugText += FString::Printf(TEXT("[GameDebugMenu] Input Current Group: %s\n"), *GroupName);
		
		if (const TArray<TWeakObjectPtr<UInputComponent>>* Stack = ActiveInputStacks.Find(CurrentInputGroupName))
		{
			for (const TWeakObjectPtr<UInputComponent>& Comp : *Stack)
			{
				if (const UInputComponent* Input = Comp.Get())
				{
					const UObject* Outer = Input->GetOuter();
					FString OuterLabel = TEXT("(null)");
					if (IsValid(Outer))
					{
						FString AssetName;
						Outer->GetFullName(nullptr).Split(TEXT("."), nullptr, &AssetName,  ESearchCase::IgnoreCase, ESearchDir::FromEnd);
						OuterLabel = AssetName;
					}
					DebugText += FString::Printf(TEXT("    > %s\n"), *OuterLabel);
				}
			}
		}

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
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

void UGDMInputSystemComponent::Initialize(UGameDebugMenuManagerAsset* MenuDataAsset)
{
	AddInputMappingContextWhenCreateManager = MenuDataAsset->AddInputMappingContextWhenCreateManager;
	AddInputMappingContextWhenDebugMenuIsShow = MenuDataAsset->AddInputMappingContextWhenDebugMenuIsShow;

	CreateDebugCameraInputClass(MenuDataAsset->DebugCameraInputClass);
	
	TArray<APlayerController*> PCs = GetPlayerControllers();
	for (APlayerController* PC : PCs)
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			for (const auto& Data : AddInputMappingContextWhenCreateManager)
			{
				if (!Subsystem->HasMappingContext(Data.InputMappingContext))
				{
					Subsystem->AddMappingContext(Data.InputMappingContext, Data.Priority);
				}
			}
		}
	}
	
	RootWidgetInputComponent = GetOwnerGameDebugMenuManager()->GetDebugMenuRootWidget()->GetMyInputComponent();
}

void UGDMInputSystemComponent::RegisterInputComponent(UInputComponent* InputComponent)
{
	/* 現在指定されたグループに対してコンポーネントを処理する */
	RegisterInputComponentToGroup(InputComponent, CurrentInputGroupName);
}

void UGDMInputSystemComponent::UnregisterInputComponent(UInputComponent* InputComponent)
{
	/* 現在指定されたグループに対してコンポーネントを処理する */
	UnregisterInputComponentFromGroup(InputComponent, CurrentInputGroupName);
}

void UGDMInputSystemComponent::RegisterInputComponentToGroup(UInputComponent* InputComponent, const FName GroupName)
{
	if ( !IsValid(InputComponent) )
	{
		UE_LOG(LogGDM, Verbose, TEXT("RegisterInputComponent failed: Not found InputComponent"));
		return;
	}

	if ( InputComponent == RootWidgetInputComponent )
	{
		UE_LOG(LogGDM, Verbose, TEXT("RegisterInputComponent failed: RootWidgetInputComponent"));
		return;
	}
	
	if (GroupName.IsNone())
	{
		UE_LOG(LogGDM, Warning, TEXT("RegisterInputComponent failed: GroupName is None"));
		return;
	}

	TArray<TWeakObjectPtr<UInputComponent>>& Group = RegisteredInputGroups.FindOrAdd(GroupName);
	if (Group.Contains(InputComponent))
	{
		/* 既に登録済みであればその場合、なにもしない */
		return;
	}
	
	Group.Add(InputComponent);

	if (GroupName == CurrentInputGroupName && bMenuOpen)
	{
		/* 現在のグループを指定し、メニューが開いていればコントローラーに追加 */
		TArray<APlayerController*> PCs = GetPlayerControllers();
		for (APlayerController* PC : PCs)
		{
			PC->PushInputComponent(InputComponent);
		}
		
		ActiveInputStacks.FindOrAdd(GroupName).AddUnique(InputComponent);
	}
}

void UGDMInputSystemComponent::UnregisterInputComponentFromGroup(UInputComponent* InputComponent, const FName GroupName)
{
	if ( !IsValid(InputComponent) )
	{
		UE_LOG(LogGDM, Verbose, TEXT("UnregisterInputComponent failed: Not found InputComponent"));
		return;
	}

	if (InputComponent == RootWidgetInputComponent)
	{
		UE_LOG(LogGDM, Verbose, TEXT("UnregisterInputComponent failed: RootWidgetInputComponent"));
		return;
	}
	
	if (GroupName.IsNone())
	{
		UE_LOG(LogGDM, Warning, TEXT("UnregisterInputComponent failed: GroupName is None"));
		return;
	}

	if (RegisteredInputGroups.Contains(GroupName))
	{
		RegisteredInputGroups[GroupName].Remove(InputComponent);
	}

	if (ActiveInputStacks.Contains(GroupName))
	{
		if (ActiveInputStacks[GroupName].Remove(InputComponent) > 0)
		{
			TArray<APlayerController*> PCs = GetPlayerControllers();
			for (APlayerController* PC : PCs)
			{
				PC->PopInputComponent(InputComponent);
			}
		}
	}
}

void UGDMInputSystemComponent::SwitchToInputGroup(const FName NewGroupName)
{
	if (NewGroupName == CurrentInputGroupName)
	{
		return;
	}

	TArray<APlayerController*> PCs = GetPlayerControllers();
	
	/* 現在のグループのInputComponentを除外 */
	if (TArray<TWeakObjectPtr<UInputComponent>>* Stack = ActiveInputStacks.Find(CurrentInputGroupName))
	{
		for (const auto& Comp : *Stack)
		{
			if (Comp.IsValid())
			{
				for (APlayerController* PC : PCs)
				{
					PC->PopInputComponent(Comp.Get());
				}
			}
		}
		
		Stack->Reset();
	}
	
	CurrentInputGroupName = NewGroupName;

	/* 新しいグループのInputComponentを追加 */
	if (const TArray<TWeakObjectPtr<UInputComponent>>* NewStack = RegisteredInputGroups.Find(NewGroupName))
	{
		for (const auto& Comp : *NewStack)
		{
			if (Comp.IsValid())
			{
				for (APlayerController* PC : PCs)
				{
					PC->PushInputComponent(Comp.Get());
				}
				
				ActiveInputStacks.FindOrAdd(NewGroupName).AddUnique(Comp);
			}
		}
	}
}

void UGDMInputSystemComponent::OnOpenMenu()
{
	if (bMenuOpen)
	{
		return;
	}

	bMenuOpen = true;

	const TArray<TWeakObjectPtr<UInputComponent>>* Group = RegisteredInputGroups.Find(CurrentInputGroupName);
	if (Group != nullptr)
	{
		for (const auto& Comp : *Group)
		{
			if (Comp.IsValid())
			{
				ActiveInputStacks.FindOrAdd(CurrentInputGroupName).AddUnique(Comp);
			}
		}
	}
	
	TArray<APlayerController*> PCs = GetPlayerControllers();
	for (APlayerController* PC : PCs)
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			for (const auto& Data : AddInputMappingContextWhenDebugMenuIsShow)
			{
				if (!Subsystem->HasMappingContext(Data.InputMappingContext))
				{
					Subsystem->AddMappingContext(Data.InputMappingContext, Data.Priority);
				}
			}
		}

		PC->PushInputComponent(RootWidgetInputComponent);
		
		if (Group != nullptr)
		{
			for (const auto& Comp : *Group)
			{
				if (Comp.IsValid())
				{
					PC->PushInputComponent(Comp.Get());
				}
			}
		}
	}
}

void UGDMInputSystemComponent::OnCloseMenu()
{
	if (!bMenuOpen)
	{
		return;
	}
	
	bMenuOpen = false;

	TArray<TWeakObjectPtr<UInputComponent>>* Stack = ActiveInputStacks.Find(CurrentInputGroupName);
	TArray<APlayerController*> PCs = GetPlayerControllers();
	for (APlayerController* PC : PCs)
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			for (const auto& Data : AddInputMappingContextWhenDebugMenuIsShow)
			{
				Subsystem->RemoveMappingContext(Data.InputMappingContext);
			}
		}

		if (Stack != nullptr)
		{
			for (const auto& Comp : *Stack)
			{
				if (Comp.IsValid())
				{
					PC->PopInputComponent(Comp.Get());
				}
			}
		}

		PC->PopInputComponent(RootWidgetInputComponent);
	}

	if (Stack != nullptr)
	{
		Stack->Reset();
	}
}

void UGDMInputSystemComponent::CreateDebugCameraInputClass(TSubclassOf<AGDMDebugCameraInput> DebugCameraInputClass)
{
	if(!IsValid(DebugCameraInputClass))
	{
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	DebugCameraInput = GetWorld()->SpawnActor<AGDMDebugCameraInput>(DebugCameraInputClass, SpawnInfo);

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
		ActorSpawnedDelegateHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UGDMInputSystemComponent::OnActorSpawned));
	}
}

AGameDebugMenuManager* UGDMInputSystemComponent::GetOwnerGameDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

TArray<APlayerController*> UGDMInputSystemComponent::GetPlayerControllers() const
{
	TArray<APlayerController*> ReturnValues;
	
	APlayerController* TargetPC = GetOwnerGameDebugMenuManager()->GetOwnerPlayerController();
	ReturnValues.Add(TargetPC);
	
	if (IsValid(TargetPC->CheatManager))
	{
		if (ADebugCameraController* DCC = TargetPC->CheatManager->DebugCameraControllerRef)
		{
			/* デバックカメラ側も対象に含める */
			ReturnValues.Add(DCC);
		}
	}
	
	return ReturnValues;
}

void UGDMInputSystemComponent::OnActorSpawned(AActor* SpawnActor)
{
	DebugCameraController = Cast<ADebugCameraController>(SpawnActor);
	if (!DebugCameraController.IsValid())
	{
		return;
	}

	GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);

	DebugCameraInput->SetDebugCameraController(DebugCameraController.Get());

	/* DebugCamera操作中にメニュー操作できるように有効化 */
	{
		DebugCameraInput->EnableInput(DebugCameraController.Get());
		GetOwnerGameDebugMenuManager()->EnableInput(DebugCameraController.Get());
	}

	/* 生成直後はローカルプレイヤーが取得できないので次回フレームに */
	TWeakObjectPtr<UGDMInputSystemComponent> WeakThis = this;
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [WeakThis]
	{
		if(WeakThis.IsValid())
		{
			if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(WeakThis->DebugCameraController->GetLocalPlayer()))
			{
				for (const auto& Data : WeakThis->AddInputMappingContextWhenCreateManager)
				{
					if (!Subsystem->HasMappingContext(Data.InputMappingContext))
					{
						Subsystem->AddMappingContext(Data.InputMappingContext, Data.Priority);
					}
				}
			}
		}
	}));
}
