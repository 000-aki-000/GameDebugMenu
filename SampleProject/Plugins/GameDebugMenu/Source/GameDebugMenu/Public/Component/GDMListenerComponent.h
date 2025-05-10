/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GDMListenerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGDMGameDebugMenuListenerDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGDMOnExecuteConsoleCommandDelegate, const FString&, Command);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMOnExecuteProcessEventDelegate, const FName&, FunctionName, UObject*, FunctionOwnerObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyBoolDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, bool, New, bool, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyIntDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, int32, New, int32, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyFloatDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, float, New, float, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyByteDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, uint8, New, uint8, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyStringDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, FString, New, FString, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyVectorDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, FVector, New, FVector, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyVector2DDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, FVector2D, New, FVector2D, Old, const FString&, PropertySaveKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGDMOnChangePropertyRotatorDelegate, const FName&, PropertyName, UObject*, PropertyOwnerObject, FRotator, New, FRotator, Old, const FString&, PropertySaveKey);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGDMOnInputSystemDelegate, UObject*, TargetInputObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMOnInputSystemChangeInputObjectDelegate, UObject*, NewInputObject, UObject*, OldInputObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGDMOnChangeDebugMenuLanguageDelegate, const FName&, NewLanguageKey, const FName&, OldLanguageKey);

/**
* DebugMenuでのイベントを取得できるコンポーネント
*/
UCLASS(Blueprintable, ClassGroup = (GameDebugMenu), hidecategories = Object, meta = (BlueprintSpawnableComponent))
class GAMEDEBUGMENU_API UGDMListenerComponent : public UActorComponent
{
	GENERATED_BODY()

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

	/** DebugMenuに登録されたプロパティ（Vector）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyVectorDelegate OnChangePropertyVectorDispatcher;

	/** DebugMenuに登録されたプロパティ（Vector2D）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyVector2DDelegate OnChangePropertyVector2DDispatcher;

	/** DebugMenuに登録されたプロパティ（Rotator）が変更されたとき呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangePropertyRotatorDelegate OnChangePropertyRotatorDispatcher;

	/** DebugMenuの使用言語が変更されたときに呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMOnChangeDebugMenuLanguageDelegate OnChangeDebugMenuLanguageDispatcher;

	/** DebugMenuでスクショ処理の開始時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnStartScreenshotRequestDispatcher;

	/** DebugMenuでスクショ処理の終了時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnScreenshotRequestProcessedDispatcher;
	
	/** DebugMenuでロードが完了した時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnLoadedDebugMenuDispatcher;
	
	/** DebugMenuでセーブが完了した時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnSavedDebugMenuDispatcher;

	/** DebugMenuのセーブが削除した時に呼ばれるイベント */
	UPROPERTY(BlueprintAssignable, Category = "GDM|Dispatcher")
	FGDMGameDebugMenuListenerDelegate OnDeletedDebugMenuDispatcher;
	
public:	
	UGDMListenerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
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
