/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Data/GameDebugMenuMasterAsset.h"

UGameDebugMenuMasterAsset::UGameDebugMenuMasterAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DebugMenuManagerClasses()
	, DebugMenuInputComponentClass(nullptr)
	, DebugReportRequesterClass()
	, GameDebugMenuStringTables()
	, FontName(nullptr)
{
}

FPrimaryAssetId UGameDebugMenuMasterAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetPrimaryType(), GetFName());
}

const FPrimaryAssetType& UGameDebugMenuMasterAsset::GetPrimaryType()
{
	static const FPrimaryAssetType AssetType = TEXT("GameDebugMenuMaster");
	return AssetType;
}

TSoftClassPtr<AGameDebugMenuManager> UGameDebugMenuMasterAsset::GetGameDebugMenuManagerSoftClass(FString ClassName) const
{
	for (const TSoftClassPtr<AGameDebugMenuManager>& ClassPtr : DebugMenuManagerClasses)
	{
		if (ClassPtr.IsNull())
		{
			continue;
		}

		FString AssetName = ClassPtr.GetAssetName();
		AssetName.RemoveFromEnd(TEXT("_C"));
		
		if (AssetName == ClassName)
		{
			return ClassPtr;
		}
	}

	return nullptr;
}
