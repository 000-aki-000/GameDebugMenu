/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameDebugMenuTypes.h"
#include "GameDebugMenuManagerAsset.generated.h"

class AGameDebugMenuManager;

/**
 * 
 */
UCLASS(Const)
class GAMEDEBUGMENU_API UGameDebugMenuManagerAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** デバックメニュー管理クラス */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GDM")
	TSubclassOf<AGameDebugMenuManager> DebugMenuManagerClass;

	/** デバックメニュー用StringTable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Localization")
	TMap<FName, FGDMStringTableList> GameDebugMenuStringTables;
	
public:
	UGameDebugMenuManagerAsset();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	FString GetDebugMenuString(const FName& LanguageKey, const FString& StringKey) const;
};
