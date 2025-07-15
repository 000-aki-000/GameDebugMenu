/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "GDMPadInputWidgetController.generated.h"

class UGDMListenerComponent;
class UGameDebugMenuWidget;

/**
* DebugMenuの中で、パッド操作などによって「選択状態」の Widget を制御するためのクラス
* 主にリスト表記のUIでの選択位置や見た目の更新を行うためのインターフェースを提供します
* 詳細はBP_GDM_PadInputWidgetController,WB_GDM_ReportMenu,WB_GDM_FavoriteMenu参照
*/
UCLASS(Blueprintable)
class GAMEDEBUGMENU_API UGDMPadInputWidgetController : public UObject
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn))
	UGameDebugMenuWidget* OwnerGameDebugMenuWidget;

public:
	virtual UWorld* GetWorld() const override;

	/**
	 * 次のWidgetを「選択中」にする
	 * @return 現在の選択中インデックスを返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 NextChoosing();

	/**
	 * １つ前のWidgetを「選択中」にする
	 * @return 現在の選択中インデックスを返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 PreviousChoosing();

	/**
	 * 現在「選択中」のWidgetを取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	UWidget* GetChoosingWidget() const;

	/**
	 * 現在「選択中」のWidgetのインデックスを取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 GetChoosingWidgetIndex() const;

	/**
	 * 現在「選択した」のWidgetを取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	UWidget* GetChosenWidget() const;

	/**
	 * 現在「選択した」のWidgetのインデックスを取得する
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 GetChosenWidgetIndex() const;

	/**
	 * 選択が確定された状態かどうかを返す
	 * @return true: 選択した状態 false: 選択中
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	bool IsChosen() const;

	/**
	 * 選択状態を「選択確定（Chosen）」に切り替える
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void ChangeChosenMode();

	/**
	 * 選択状態を「選択中（Choosing）」に切り替える
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void ChangeChoosingMode();

	/**
	 * 新しく「選択中」とする Widget を直接指定
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	void SetChoosingWidget(UWidget* NewChoosingWidget);

	/**
	 * 指定した UWidget が内部で管理していれていれば、そのインデックスを返す
	 * @param TargetWidget - 対象のWidget 
	 * @return あれば 0 以上,なければ -1を返す  
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintImplementableEvent, Category = "GDM")
	int32 GetWidgetIndexByWidget(UWidget* TargetWidget) const;

	/**
	 * 内部で管理している Widget 群の中から、インデックスに対応する Widget を取得
	 * @param Index - 対象のインデックス
	 * @return 無効なインデックスなら nullptr を返す
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintImplementableEvent, Category = "GDM")
	UWidget* GetWidgetByIndex(int32 Index) const;

	/**
	 * 管理してるWidgetの数を取得
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "GDM")
	int32 GetWidgetCount() const;
};
