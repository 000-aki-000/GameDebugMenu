/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameDebugMenuTypes.h"
#include "GDMPlayerControllerProxyComponent.generated.h"

/**
* DebugMenuManager生成時、PlayerControllerに自動で追加されるコンポーネント
* 使用用途としてはマルチプレイなどでDebug目的の通信が必要な場合、PlayerControllerの代わりに記述などができる
*/
UCLASS(Blueprintable, NotBlueprintType, noteditinlinenew)
class GAMEDEBUGMENU_API UGDMPlayerControllerProxyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGDMPlayerControllerProxyComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UFUNCTION(BlueprintPure)
	APlayerController* GetOwnerPlayerController();

	UFUNCTION(BlueprintPure)
	APawn* GetOwnerPlayerPawn();

	UFUNCTION(BlueprintPure)
	ACharacter* GetOwnerPlayerCharacter();

	UFUNCTION(Server, Reliable, WithValidation)
	void ROS_ExecuteConsoleCommand(const FString& Command,bool bAllClient);

	UFUNCTION(Client, Reliable, WithValidation)
	void ROC_ExecuteConsoleCommand(const FString& Command);

	virtual void ExecuteConsoleCommand(const FString& Command, EGDMConsoleCommandNetType CommandNetType);
	virtual void AllExecuteConsoleCommand_Server(const FString& Command);
};
