/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuWidget.h"
#include "GameDebugMenuRootWidget.generated.h"

class AGameDebugMenuManager;

/**
 * DebugMenuのルートに当たるWidget
 */
UCLASS()
class GAMEDEBUGMENU_API UGameDebugMenuRootWidget : public UGameDebugMenuWidget
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<AGameDebugMenuManager> Manager = nullptr;
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void ActivateDebugMenu() override;
	virtual void DeactivateDebugMenu() override;

public:
	void SetDebugMenuManager(AGameDebugMenuManager* InManager);

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeRootWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDebugReport();
	
	UFUNCTION(BlueprintPure)
	AGameDebugMenuManager* GetOwnerManager() const;

	UFUNCTION(BlueprintCallable)
	void SwitchInputComponentGroupForGameDebugMenu(FName NewGroupName);
};
