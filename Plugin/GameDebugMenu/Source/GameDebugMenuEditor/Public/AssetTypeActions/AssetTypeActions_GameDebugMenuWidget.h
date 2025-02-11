/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_GameDebugMenuWidget : public FAssetTypeActions_Base
{
public:
	/* Begin IAssetTypeActions */
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual bool CanLocalize() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	/* End IAssetTypeActions */
};
