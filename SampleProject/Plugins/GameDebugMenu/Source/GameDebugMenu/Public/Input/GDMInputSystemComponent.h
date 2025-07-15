/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Data/GameDebugMenuManagerAsset.h"
#include "GDMInputSystemComponent.generated.h"

class ADebugCameraController;
class UGDMEnhancedInputComponent;
class AGameDebugMenuManager;
class AGDMDebugCameraInput;

/**
* DebugMenuでの入力処理を管理するコンポーネント
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

	/** 登録中のInputComponent郡 */
	TMap<FName, TArray<TWeakObjectPtr<UInputComponent>>> RegisteredInputGroups;

	/** 利用中のInputComponent郡 */
	TMap<FName, TArray<TWeakObjectPtr<UInputComponent>>> ActiveInputStacks;

	/** 現在アクティブなInputComponentのグループ名 */
	FName CurrentInputGroupName;
	
	/** メニューの開閉状態の識別フラグ */
	UPROPERTY(Transient)
	bool bMenuOpen;

	/** ルートのWidgetが持つInputComponent */
	UPROPERTY(Transient)
	TObjectPtr<UGDMEnhancedInputComponent> RootWidgetInputComponent;

	/** DebugMenuManagerが作られたときに追加される InputMappingContext */
	UPROPERTY(Transient)
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenCreateManager;
	 
	/** DebugMenuを表示するときに追加される InputMappingContext */
	UPROPERTY(Transient)
	TArray<FGameDebugMenuWidgetInputMappingContextData> AddInputMappingContextWhenDebugMenuIsShow;

	/** ADebugCameraController生成チェック用のハンドル */
	FDelegateHandle ActorSpawnedDelegateHandle;

	TWeakObjectPtr<ADebugCameraController> DebugCameraController;
	
public:
	/** 入力判定用ログ */
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

	/**
	 * 初期化
	 */
	virtual void Initialize(UGameDebugMenuManagerAsset* MenuDataAsset);

	/**
	 * 現在アクティブなグループにInputComponentを登録/解除をする
	 */
	virtual void RegisterInputComponent(UInputComponent* InputComponent);
	virtual void UnregisterInputComponent(UInputComponent* InputComponent);
	
	/**
	 * 指定したグループにInputComponentを登録/解除をする
	 */
	virtual void RegisterInputComponentToGroup(UInputComponent* InputComponent, const FName GroupName);
	virtual void UnregisterInputComponentFromGroup(UInputComponent* InputComponent, const FName GroupName);

	/**
	 * 入力グループを変更する
	 * @param NewGroupName - 新しいグループ名
	*/
	virtual void SwitchToInputGroup(const FName NewGroupName);

	/**
	 * デバックメニューが開くと呼ばれる
	 * @note アクティブなメニューのInputComponentをPlayerControllerに追加する
	 */
	virtual void OnOpenMenu();

	/**
	 * デバックメニューが閉じると呼ばれる
	 * @note アクティブなメニューのInputComponentをPlayerControllerに削除する
	 */
	virtual void OnCloseMenu();

protected:
	virtual void CreateDebugCameraInputClass(TSubclassOf<AGDMDebugCameraInput> DebugCameraInputClass);
	AGameDebugMenuManager* GetOwnerGameDebugMenuManager() const;
	TArray<APlayerController*> GetPlayerControllers() const;
	virtual void OnActorSpawned(AActor* SpawnActor);
};
