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

USTRUCT(BlueprintType)
struct FGDMConsoleCommandProviderPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Pattern;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite)
	TObjectPtr<UGDMConsoleCommandValueProvider> Provider = nullptr;
};

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
