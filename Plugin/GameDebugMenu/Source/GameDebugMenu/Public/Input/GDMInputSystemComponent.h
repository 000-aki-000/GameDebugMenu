/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Data/GameDebugMenuDataAsset.h"
#include "GDMInputSystemComponent.generated.h"

class ADebugCameraController;
class UInputComponent;
class AGameDebugMenuManager;
class AGDMDebugCameraInput;

/**
* DebugMenuでの入力処理を行うコンポーネント
*/
UCLASS()
class GAMEDEBUGMENU_API UGDMInputSystemComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/** DebugMenuの入力を無視するフラグカウント */
	uint16 IgnoreDebugMenuInput;
	
	/** デバックカメラ用のインプットアクターのインスタンス */
	UPROPERTY(Transient)
	TObjectPtr<AGDMDebugCameraInput> DebugCameraInput;

	/**  */
	TMap<FName, TArray<TWeakObjectPtr<UInputComponent>>> RegisteredInputGroups;

	/**  */
	TMap<FName, TArray<TWeakObjectPtr<UInputComponent>>> ActiveInputStacks;

	/**  */
	FName CurrentInputGroupName;
	
	/**  */
	UPROPERTY(Transient)
	bool bMenuOpen;

	/**  */
	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> RootWidgetInputComponent;

	/** DebugMenuManagerが作られたときに追加される InputMappingContext */
	UPROPERTY(Transient)
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenCreateManager;
	 
	/** DebugMenuを表示するときに追加される InputMappingContext */
	UPROPERTY(Transient)
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenDebugMenuIsShow;

	FDelegateHandle ActorSpawnedDelegateHandle;

	TWeakObjectPtr<ADebugCameraController> DebugCameraController;
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Debug")
	bool bOutputDebugLog;

public:	
	UGDMInputSystemComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void SetIgnoreInput(bool bNewInput);

	virtual void ResetIgnoreInput();

	virtual bool IsInputIgnored() const;

	virtual void Initialize(UGameDebugMenuDataAsset* MenuDataAsset);

	virtual void RegisterInputComponent(UInputComponent* InputComponent);
	
	virtual void UnregisterInputComponent(UInputComponent* InputComponent);
	
	virtual void RegisterInputComponentToGroup(UInputComponent* InputComponent, const FName GroupName);
	virtual void UnregisterInputComponentFromGroup(UInputComponent* InputComponent, const FName GroupName);

	void SwitchToInputGroup(const FName NewGroupName);
	
	// メニューを開く
	virtual void OnOpenMenu();

	// メニューを閉じる
	virtual void OnCloseMenu();

protected:
	
	virtual void CreateDebugCameraInputClass(TSubclassOf<AGDMDebugCameraInput> DebugCameraInputClass);
	
	AGameDebugMenuManager* GetOwnerGameDebugMenuManager() const;

	TArray<APlayerController*> GetPlayerControllers() const;

	virtual void OnActorSpawned(AActor* SpawnActor);
};
