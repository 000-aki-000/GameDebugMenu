/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMDebugCameraInput.h"
#include "Engine/World.h"
#include "Engine/DebugCameraController.h"
#include "GameDebugMenuManager.h"
#include <GameDebugMenuFunctions.h>

AGDMDebugCameraInput::AGDMDebugCameraInput()
	: Super()
	, DebugCameraController(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

AGameDebugMenuManager* AGDMDebugCameraInput::GetOwnerGameDebugMenuManager() const
{
	return Cast<AGameDebugMenuManager>(GetOwner());
}

ADebugCameraController* AGDMDebugCameraInput::GetDebugCameraController() const
{
	return DebugCameraController.Get();
}

void AGDMDebugCameraInput::SetDebugCameraController(ADebugCameraController* DCC)
{
	DebugCameraController = DCC;
}

void AGDMDebugCameraInput::ToggleOrbitHitPoint()
{
	UE_LOG(LogGDM, Log, TEXT("AGDMDebugCameraInput::ToggleOrbitHitPoint Call"));

	AGameDebugMenuManager* DebugMenuManager = GetOwnerGameDebugMenuManager();
	if (!IsValid(DebugMenuManager))
	{
		return;
	}

	if (DebugMenuManager->IsInputIgnored())
	{
		return;
	}
	
	if(DebugCameraController.IsValid())
	{
		DebugCameraController->ToggleOrbitHitPoint();
	}
}

void AGDMDebugCameraInput::PawnTeleport()
{
	UE_LOG(LogGDM, Log, TEXT("AGDMDebugCameraInput::PawnTeleport Call"));
	
	AGameDebugMenuManager* DebugMenuManager = GetOwnerGameDebugMenuManager();
	if (!IsValid(DebugMenuManager))
	{
		return;
	}

	if (DebugMenuManager->IsInputIgnored())
	{
		return;
	}

	DebugMenuManager->ExecuteConsoleCommand(TEXT("Teleport"), DebugMenuManager->GetOwnerPlayerController());
}
