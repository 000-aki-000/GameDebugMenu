/**
* Copyright (c) 2021 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "EngineMinimal.h"
#include "GameDebugMenuWidget.h"
#include "GameDebugMenuTypes.h"
#include "GDMPropertyWidget.generated.h"

/**
 * 
 */
UCLASS()
class UGDMPropertyWidget : public UGameDebugMenuWidget
{
	GENERATED_BODY()

public:

	/** プロパティ所持オブジェクト */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	UObject* TargetObject;

	/** プロパティ名 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FName PropertyName;

	/** プロパティの種類 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	EGDMPropertyType PropertyType;

	/** プロパティの種類がEnumだった場合の名前 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FName EnumName;

	/** UIの設定情報 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FGDMPropertyUIConfigInfo PropertyConfigInfo;

	/** UI操作時のプロパティの変化量 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	float ChangeAmount;

protected:
	bool bStartChangeAmount;
	bool bChangedMaxChangeAmount;
	float ElapsedTime;

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "GDM")
	virtual void StartChangeAmountTime();

	UFUNCTION(BlueprintCallable, Category = "GDM")
	virtual void ResetChangeAmountTime();

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	bool GetPropertyValue_Bool(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Bool(bool bNewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	float GetPropertyValue_Float(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Float(float NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	int32 GetPropertyValue_Int(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Int(int32 NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	uint8 GetPropertyValue_Byte(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Byte(uint8 NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	TArray<FText> GetEnumDisplayNames(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	FString GetPropertyValue_String(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_String(FString NewValue, bool& bHasProperty);

};