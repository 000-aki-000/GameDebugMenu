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
#include "GameDebugMenuMasterAsset.generated.h"

class AGDMDebugReportRequester;
class UGDMEnhancedInputComponent;
class AGameDebugMenuManager;

/**
 * デバッグメニュー全体で使用されるアセット情報をまとめたアセット
 * デバッグメニューに関係する構成要素への参照を一元化して保持
 * UPrimaryDataAsset なのでPrimaryAssetTypesToScanに追加が必要。（GetPrimaryType参照）
 * UGameDebugMenuSettingsのMasterAssetNameにこのアセットを指定することで使用できる
 */
UCLASS(Const)
class GAMEDEBUGMENU_API UGameDebugMenuMasterAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 使用するマネージャークラス */
	UPROPERTY(EditAnywhere, Category = "Manager")
	TArray<TSoftClassPtr<AGameDebugMenuManager>> DebugMenuManagerClasses;

	/** メニューの入力処理をするコンポーネント */
	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftClassPtr<UGDMEnhancedInputComponent> DebugMenuInputComponentClass;

	/** バグレポート処理クラス */
	UPROPERTY(EditAnywhere, Category = "ReportSettings")
	TMap<EGDMProjectManagementTool, TSubclassOf<AGDMDebugReportRequester>> DebugReportRequesterClass;
	
	/** デバックメニュー用StringTable */
	UPROPERTY(EditAnywhere, Category = "Localization")
	TMap<FName, FGDMStringTableList> GameDebugMenuStringTables;

	/** デバックメニューのUMG使用フォント */
	UPROPERTY(EditAnywhere, Category = "Font", meta = (AllowedClasses = "/Script/Engine.Font", DisplayName = "Font Family"))
	FSoftObjectPath FontName;

public:
	UGameDebugMenuMasterAsset(const FObjectInitializer& ObjectInitializer);
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	static const FPrimaryAssetType& GetPrimaryType();


	TSoftClassPtr<AGameDebugMenuManager> GetGameDebugMenuManagerSoftClass(FString ClassName) const;
};

