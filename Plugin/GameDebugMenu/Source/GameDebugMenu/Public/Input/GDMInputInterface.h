/**
* Copyright (c) 2022 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameDebugMenuTypes.h"
#include "GDMInputInterface.generated.h"


UINTERFACE(MinimalAPI)
class UGDMInputInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * DebugMenu操作用入力インターフェイス
 */
class GAMEDEBUGMENU_API IGDMInputInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputAxisMoveForwardGDM(float Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputAxisMoveRightGDM(float Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputAxisLookRightGDM(float Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputAxisLookUpGDM(float Value);


	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputPressedGDM(FName InputEventName, bool bRepeat);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnInputReleasedGDM(FName InputEventName);

	/**
	* DebugMenuの入力処理の（無）有効判定
	*
	* @return True : 入力処理をする, False : 入力処理をしない
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,Category = "GDM|InputInterface")
	bool IsEnableInputGDM();
	virtual bool IsEnableInputGDM_Implementation();

	/**
	* モーダルウィンドウのように入力処理の振る舞いを変えるフラグ。IsEnableInputGDMがFalseのとき動作が変わる
	* 
	* @return true : IsEnableInputGDMがFalseになっても優先度が下のオブジェクトに入力処理が映らなくなる False : 自分より優先度の低いオブジェクトに入力処理が移る
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent,Category = "GDM|InputInterface")
	bool IsModalModeGDM();

	/**
	* 入力処理の自身の優先度。数字が大きいほど優先度があがる
	*/
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "GDM|InputInterface")
	int32 GetInputPriorityGDM();
	virtual int32 GetInputPriorityGDM_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnRegisterGDMInputSystem();

	UFUNCTION(BlueprintImplementableEvent, Category = "GDM|InputInterface")
	void OnUnregisterGDMInputSystem();
};
