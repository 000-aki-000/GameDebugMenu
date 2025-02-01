/**
* Copyright (c) 2025 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Component/GDMPlayerControllerProxyComponent.h"
#include <Engine/World.h>
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameDebugMenuFunctions.h"

UGDMPlayerControllerProxyComponent::UGDMPlayerControllerProxyComponent()
{
	PrimaryComponentTick.bCanEverTick          = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UGDMPlayerControllerProxyComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogGDM, Log, TEXT("Call BeginPlay Spawn GDMPlayerControllerProxyComponent"));
}

void UGDMPlayerControllerProxyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogGDM, Log, TEXT("Call EndPlay Destroyed GDMPlayerControllerProxyComponent"));
}

APlayerController* UGDMPlayerControllerProxyComponent::GetOwnerPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

APawn* UGDMPlayerControllerProxyComponent::GetOwnerPlayerPawn() const
{
	const APlayerController* PlayerController = GetOwnerPlayerController();
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}

	return PlayerController->GetPawn();
}

ACharacter* UGDMPlayerControllerProxyComponent::GetOwnerPlayerCharacter() const
{
	const APlayerController* PlayerController = GetOwnerPlayerController();
	if (IsValid(PlayerController))
	{
		return nullptr;
	}

	return PlayerController->GetCharacter();
}

void UGDMPlayerControllerProxyComponent::ROS_ExecuteConsoleCommand_Implementation(const FString& Command, bool bAllClient)
{
	if (bAllClient)
	{
		AllExecuteConsoleCommand_Server(Command);
	}
	else
	{
		ROC_ExecuteConsoleCommand(Command);
	}
}

bool UGDMPlayerControllerProxyComponent::ROS_ExecuteConsoleCommand_Validate(const FString& Command, bool bAllClient)
{
	return (!Command.IsEmpty());
}

void UGDMPlayerControllerProxyComponent::ROC_ExecuteConsoleCommand_Implementation(const FString& Command)
{
	UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->ExecuteConsoleCommand(Command, Cast<APlayerController>(GetOwner()));
}

bool UGDMPlayerControllerProxyComponent::ROC_ExecuteConsoleCommand_Validate(const FString& Command)
{
	return (!Command.IsEmpty());
}

void UGDMPlayerControllerProxyComponent::ExecuteConsoleCommand(const FString& Command, EGDMConsoleCommandNetType CommandNetType)
{
	switch (CommandNetType)
	{
		case EGDMConsoleCommandNetType::LocalOnly:
		{
			/* 通信せず実行者の環境で実行する */
			UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->ExecuteConsoleCommand(Command, Cast<APlayerController>(GetOwner()));
			break;
		}
		case EGDMConsoleCommandNetType::ServerAll:
		{
			/* Serverで全プレイヤーに実行 */
			if (GetOwner()->HasAuthority())
			{
				AllExecuteConsoleCommand_Server(Command);
			}
			else
			{
				ROS_ExecuteConsoleCommand(Command,true);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void UGDMPlayerControllerProxyComponent::AllExecuteConsoleCommand_Server(const FString& Command)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (!IsValid(PC))
		{
			continue;
		}

		UGameDebugMenuFunctions::GetGameDebugMenuManager(this)->ExecuteConsoleCommand(Command, PC);
	}
}
