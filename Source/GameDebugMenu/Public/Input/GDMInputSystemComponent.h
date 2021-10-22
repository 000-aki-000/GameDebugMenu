/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMInputSystemComponent.generated.h"

class UInputComponent;
class AGameDebugMenuManager;

/**
* DebugMenuでの入力処理を行うコンポーネント
*/
UCLASS()
class GAMEDEBUGMENU_API UGDMInputSystemComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/**
	* デバックメニューの入力リピート処理
	*/
	class RepeatButtonInfo
	{
	private:
		typedef void (UGDMInputSystemComponent::* FUNCTION)(const FName&);
		UGDMInputSystemComponent* Instance;
		FUNCTION CallFunction;
		bool bRun;
		float ElapsedTime;

	public:
		FName EventName;

	public:
		RepeatButtonInfo();
		RepeatButtonInfo(UGDMInputSystemComponent* Ins);
		void SetFunction(FUNCTION Function);
		void Tick(float DeltaTime);
		void Start();
		void Stop();
	};


	/** 各種キーのリピート処理 */
	TMap< FName, RepeatButtonInfo > RepeatButtons;

	/** 登録済みデバックメニューのインプット対象オブジェクト */
	TArray<TWeakObjectPtr<UObject>> RegisterInputObjects;

	/** 現在入力処理対象のオブジェクト */
	TWeakObjectPtr<UObject> CurrentInputObject;

	/** 入力中の各イベントごとのオブジェクト */
	TMap< FName, TWeakObjectPtr<UObject> > CachePressedInputObjects;

	/** True: InputComponentのバインド完了 */
	bool bBindingsInputComponent;

	/** DebugMenuの入力を無視するフラグカウント */
	uint16 IgnoreDebugMenuInput;

	/** True: マッピング追加完了 */
	bool bActionMappingBindingsAdded;

public:	
	UGDMInputSystemComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/**
	* DebugMenuで使用する入力イベントをバインドする
	*/
	virtual void InitializeInputComponentBindings(UInputComponent* OwnerInputComponent);

	/**
	* DebugMenuの入力処理するオブジェクトを登録する
	*/
	virtual bool RegisterInputObject(UObject* TargetObject);

	/**
	* DebugMenuの入力処理するオブジェクトを登録解除する
	*/
	virtual bool UnregisterInputObject(UObject* TargetObject);

	virtual void SetIgnoreInput(bool bNewInput);

	virtual void ResetIgnoreInput();

	virtual bool IsInputIgnored() const;

protected:
	/**
	* DebugMenuで使用する入力イベントをセットする
	*/
	virtual void InitializeInputMapping();

	AGameDebugMenuManager* GetOwnerGameDebugMenuManager() const;
	virtual void SetupInputComponentBindActions(UInputComponent* OwnerInputComponent);
	virtual UObject* GetCurrentInputObject(bool bCheckEnableInput = true);
	virtual void UpdateInputObject();

	/************************************************************************/
	/* 押されたときの入力													*/
	/************************************************************************/
protected:
	virtual void CallInputPressedInterfaceEvent(const FName& EventName);
	virtual void OnPressedUp();
	virtual void OnPressedDown();
	virtual void OnPressedLeft();
	virtual void OnPressedRight();
	virtual void OnPressedDecide();
	virtual void OnPressedCancel();
	virtual void OnPressedMenuClose();

	/************************************************************************/
	/* 離したときの入力														*/
	/************************************************************************/
protected:
	virtual void CallInputReleasedInterfaceEvent(const FName& EventName);
	virtual void OnReleasedUp();
	virtual void OnReleasedDown();
	virtual void OnReleasedLeft();
	virtual void OnReleasedRight();
	virtual void OnReleasedDecide();
	virtual void OnReleasedCancel();
public:
	virtual void CallReleasedButtons();

	/************************************************************************/
	/* Axis系イベント														*/
	/************************************************************************/
protected:
	virtual void OnAxisMoveForward(float Value);
	virtual void OnAxisMoveRight(float Value);
	virtual void OnAxisLookRight(float Value);
	virtual void OnAxisLookUp(float Value);

	/************************************************************************/
	/* リピートイベント													*/
	/************************************************************************/
protected:
	virtual void OnInputRepeat(const FName& EventName);
	virtual void SetupRepeatButtons();
	virtual void TickRepeatButtons(const float DeltaTime);
};
