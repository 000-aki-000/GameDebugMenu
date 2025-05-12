/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDMDebugCameraInput.generated.h"

class ADebugCameraController;
class AGameDebugMenuManager;

/**
* DebugCameraController有効時に入力クラス
*/
UCLASS(Blueprintable, NotBlueprintType, notplaceable)
class GAMEDEBUGMENU_API AGDMDebugCameraInput : public AActor
{
	GENERATED_BODY()
	
	FDelegateHandle ActorSpawnedDelegateHandle;

	TWeakObjectPtr<ADebugCameraController> DebugCameraController;

public:	
	AGDMDebugCameraInput();

public:
	UFUNCTION(BlueprintPure)
	AGameDebugMenuManager* GetOwnerGameDebugMenuManager() const;

	UFUNCTION(BlueprintPure)
	ADebugCameraController* GetDebugCameraController() const;
	
	void SetDebugCameraController(ADebugCameraController* DCC);

	UFUNCTION(BlueprintCallable)
	virtual void ToggleOrbitHitPoint();

	UFUNCTION(BlueprintCallable)
	virtual void PawnTeleport();
};
