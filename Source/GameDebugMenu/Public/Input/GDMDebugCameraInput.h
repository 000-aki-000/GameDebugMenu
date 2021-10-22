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
* DebugCameraController有効時にメニューの操作をできるようにするための入力クラス
*/
UCLASS(Blueprintable, NotBlueprintType, notplaceable)
class GAMEDEBUGMENU_API AGDMDebugCameraInput : public AActor
{
	GENERATED_BODY()
	
	FDelegateHandle ActorSpawnedDelegateHandle;

	TWeakObjectPtr<ADebugCameraController> DebugCameraController;

	bool bBindingsInputComponent;

public:	
	AGDMDebugCameraInput();

public:	
	virtual void EnableInput(class APlayerController* PlayerController) override;

	virtual AGameDebugMenuManager* GetOwnerGameDebugMenuManager();

	virtual void BindSpawnDebugCameraController();

	virtual void UnbindSpawnDebugCameraController();

	virtual void OnPressedTaggleMenu();

	virtual void OnPressedDebugReport();

	virtual void ToggleOrbitHitPoint();

	virtual void OnActorSpawned(AActor* SpawnActor);

	UFUNCTION(BlueprintCallable)
	void ChangeDynamicBlockInput(bool bNewBlockInput);
};
