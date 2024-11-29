/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMDebugCameraInput.h"
#include "Engine/World.h"
#include "Engine/DebugCameraController.h"
#include "GameDebugMenuManager.h"
#include <GameDebugMenuFunctions.h>
#include "Components/InputComponent.h"
#include "Input/GDMInputEventFunctions.h"

AGDMDebugCameraInput::AGDMDebugCameraInput()
	: Super()
	, ActorSpawnedDelegateHandle()
	, DebugCameraController(nullptr)
	, bBindingsInputComponent(false)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGDMDebugCameraInput::EnableInput(class APlayerController* PlayerController)
{
	/* DebugCameraControllerに対して有効にする */
	Super::EnableInput(PlayerController);

	if (bBindingsInputComponent == false)
	{
		bBindingsInputComponent = true;

		/* メニュー開閉イベントをバインドしてDebugCamera操作中メニューを出せるように */
		FInputActionBinding* InputActionBinding = &InputComponent->BindAction(GDMInputEventNames::MenuOpenAndClose, EInputEvent::IE_Pressed, this, &AGDMDebugCameraInput::OnPressedToggleMenu);
		InputActionBinding->bExecuteWhenPaused = true;
		InputActionBinding = &InputComponent->BindAction(GDMInputEventNames::DebugReport, EInputEvent::IE_Pressed, this, &AGDMDebugCameraInput::OnPressedDebugReport);
		InputActionBinding->bExecuteWhenPaused = true;
 		InputActionBinding = &InputComponent->BindAction(GDMInputEventNames::OrbitHitPoint, IE_Pressed, this, &AGDMDebugCameraInput::ToggleOrbitHitPoint);
 		InputActionBinding->bExecuteWhenPaused = true;
	}
}

AGameDebugMenuManager* AGDMDebugCameraInput::GetOwnerGameDebugMenuManager()
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

void AGDMDebugCameraInput::BindSpawnDebugCameraController()
{
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogGDM, Warning, TEXT("AGDMDebugCameraInput::BindSpawnDebugCameraController Not Found GetWorld"));
		return;
	}

	ActorSpawnedDelegateHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &AGDMDebugCameraInput::OnActorSpawned));
}

void AGDMDebugCameraInput::UnbindSpawnDebugCameraController()
{
	if (!IsValid(GetWorld()))
	{
		UE_LOG(LogGDM, Warning, TEXT("AGDMDebugCameraInput::UnbindSpawnDebugCameraController Not Found GetWorld"));
		return;
	}

	GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
}

void AGDMDebugCameraInput::OnPressedToggleMenu()
{
	UE_LOG(LogGDM, Log, TEXT("AGDMDebugCameraInput::OnPressedToggleMenu Call"));

	AGameDebugMenuManager* DebugMenuManager = GetOwnerGameDebugMenuManager();
	if (!IsValid(DebugMenuManager))
	{
		return;
	}

	if(DebugMenuManager->IsInputIgnored())
	{
		return;
	}

	if (DebugMenuManager->IsShowingDebugMenu())
	{
		DebugMenuManager->HideDebugMenu();
	}
	else
	{
		DebugMenuManager->ShowDebugMenu();
	}
}

void AGDMDebugCameraInput::OnPressedDebugReport()
{
	UE_LOG(LogGDM, Log, TEXT("AGDMDebugCameraInput::OnPressedDebugReport Call"));

	UGameDebugMenuFunctions::ShowDebugReport(this);
}

void AGDMDebugCameraInput::ToggleOrbitHitPoint()
{
	UE_LOG(LogGDM, Log, TEXT("AGDMDebugCameraInput::ToggleOrbitHitPoint Call"));

	if(DebugCameraController.IsValid())
	{
		DebugCameraController->ToggleOrbitHitPoint();
	}
}

void AGDMDebugCameraInput::OnActorSpawned(AActor* SpawnActor)
{
	DebugCameraController = Cast<ADebugCameraController>(SpawnActor);
	if (!DebugCameraController.IsValid())
	{
		return;
	}

	/* Debugカメラが生成されたら入力を有効にする */
	UnbindSpawnDebugCameraController();
	EnableInput(DebugCameraController.Get());
}

void AGDMDebugCameraInput::ChangeDynamicBlockInput(bool bNewBlockInput)
{
	if(IsValid(InputComponent))
	{
		InputComponent->bBlockInput = bNewBlockInput;
	}
}
