/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMListenerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGDMGameDebugMenuListenerDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGDMOnExecuteConsoleCommandDelegate, const FString&, Command);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMOnExecuteProcessEventDelegate, const FName&, FunctionName, UObject*, FunctionOwnerObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGDMOnChangePropertyBoolDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, bool, New, bool, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGDMOnChangePropertyIntDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, int32, New, int32, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGDMOnChangePropertyFloatDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, float, New, float, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGDMOnChangePropertyByteDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, uint8, New, uint8, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGDMOnChangePropertyStringDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, FString, New, FString, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGDMOnInputSystemDelegate, UObject*, TargetInputObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMOnInputSystemChangeInputObjectDelegate, UObject*, NewInputObject, UObject*, OldInputObject);

/**
* DebugMenuでのイベントを取得できるコンポーネント
*/
UCLASS(Blueprintable, ClassGroup = (GameDebugMenu), hidecategories = Object, meta = (BlueprintSpawnableComponent))
class GAMEDEBUGMENU_API UGDMListenerComponent : public UActorComponent
{
	GENERATED_BODY()

	static TMap<UWorld*, TArray<UGDMListenerComponent*>> CacheListenerComponents;

public:
	/** DebugMenuが表示されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnShowDispatcher;

	/** DebugMenuが閉じたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnHideDispatcher;

	/** DebugMenuからコンソールコマンド実行した時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnExecuteConsoleCommandDelegate OnExecuteConsoleCommandDispatcher;

	/** DebugMenuに登録された関数が実行されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnExecuteProcessEventDelegate OnExecuteProcessEventDispatcher;

	/** DebugMenuに登録されたプロパティ（Bool）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyBoolDelegate OnChangePropertyBoolDispatcher;

	/** DebugMenuに登録されたプロパティ（Int）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyIntDelegate OnChangePropertyIntDispatcher;

	/** DebugMenuに登録されたプロパティ（Float）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyFloatDelegate OnChangePropertyFloatDispatcher;

	/** DebugMenuに登録されたプロパティ（Byte）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyByteDelegate OnChangePropertyByteDispatcher;

	/** DebugMenuに登録されたプロパティ（String）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyStringDelegate OnChangePropertyStringDispatcher;

	/** インプットシステムに登録されたときに呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnInputSystemDelegate OnRegisterInputSystemDispatcher;

	/** インプットシステムから解除されたときに呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnInputSystemDelegate OnUnregisterInputSystemDispatcher;

	/** インプットシステムでアクティブなオブジェクトが変更されたときに呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnInputSystemChangeInputObjectDelegate OnChangeActiveInputObjectDispatcher;

public:	
	UGDMListenerComponent();
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

public:
	UFUNCTION(BlueprintCallable)
	void AllUnbindDispatchers();

public:
	static int32 PushListenerComponent(UWorld* TargetWorld, UGDMListenerComponent* Listener);
	static int32 PopListenerComponent(UWorld* TargetWorld, UGDMListenerComponent* Listener);
	static void GetAllListenerComponents(UWorld* TargetWorld, TArray<UGDMListenerComponent*>& OutListenerComponents);
};
