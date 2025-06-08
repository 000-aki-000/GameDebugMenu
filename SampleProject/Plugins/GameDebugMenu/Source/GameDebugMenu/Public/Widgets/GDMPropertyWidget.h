/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
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
	TObjectPtr<UObject> TargetObject;

	/** プロパティ名 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FName PropertyName;

	/** プロパティの種類 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	EGDMPropertyType PropertyType;

	/** UIの設定情報 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FGDMPropertyUIConfigInfo PropertyConfigInfo;

	/** UI操作時のプロパティの変化量 */
	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	float ChangeAmount;

	UPROPERTY(BlueprintReadWrite, Category = "GDM|Properties")
	FString PropertySaveKey;
	
protected:
	bool bStartChangeAmount;
	bool bChangedMaxChangeAmount;
	float ElapsedTime;
	float InactiveElapsedTime = 0.0f;

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
	TArray<FText> GetEnumDisplayNames(const FString& EnumPath, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	FString GetPropertyValue_String(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_String(FString NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	FVector GetPropertyValue_Vector(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Vector(FVector NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	FVector2D GetPropertyValue_Vector2D(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Vector2D(FVector2D NewValue, bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	FRotator GetPropertyValue_Rotator(bool& bHasProperty);

	UFUNCTION(BlueprintCallable, Category = "GDM|Properties")
	void SetPropertyValue_Rotator(FRotator NewValue, bool& bHasProperty);

};
