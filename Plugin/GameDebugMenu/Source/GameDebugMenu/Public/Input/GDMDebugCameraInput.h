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
* DebugCameraController動作中に処理される入力判定を制御するクラス
* 「ToogleDebugCamera」実行時に別途、入力機能（メニューの開閉など）をここで実装できるようにしてる
* （BP_GDMDebugCameraInput参照）
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
