/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMConsoleCommandValueProvider.h"
#include "GDMConsoleCommandValueProviderComponent.generated.h"

class UGDMConsoleVariableCommandValueProvider;

/**
 * 特定の部分文字列（パターン）に一致するコンソールコマンドに対し、
 * 対応する ValueProvider を指定する構造体。
 */
USTRUCT(BlueprintType)
struct FGDMConsoleCommandProviderPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Pattern;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UGDMConsoleCommandValueProvider> Provider = nullptr;
};

/**
 * 登録されたパターンに応じて適切な UGDMConsoleCommandValueProvider を選択し、
 * コンソールコマンドから値を取得する ActorComponent。
 * 
 * パターンマッチが成功しない場合は ConsoleVariableCommandValueProvider を fallback として利用する。
 */
UCLASS()
class GAMEDEBUGMENU_API UGDMConsoleCommandValueProviderComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category="GDM")
	TArray<FGDMConsoleCommandProviderPattern> ProviderPatterns;

	UPROPERTY(EditAnywhere, Instanced, Category="GDM")
	TObjectPtr<UGDMConsoleVariableCommandValueProvider> ConsoleVariableCommandValueProvider;
	
public:
	UGDMConsoleCommandValueProviderComponent();

	UFUNCTION(BlueprintCallable, Category = "Console")
	bool GetFloatValue(const FString& CommandName, float& OutValue);
};
