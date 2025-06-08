/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Favorite/GDMFavoriteItemDefinition.h"

#include "GameDebugMenuManager.h"

UWorld* UGDMFavoriteItemDefinition::GetWorld() const
{
	if (UPackage* PkgObj = Cast<UPackage>(GetOuter()))
	{
		return GWorld;
	}
	
	if (OwnerManager.IsValid())
	{
		return OwnerManager.Get()->GetWorld();
	}
	
	return nullptr;
}

AGameDebugMenuManager* UGDMFavoriteItemDefinition::GetOwnerManager() const
{
	if (OwnerManager.IsValid())
	{
		return OwnerManager.Get();
	}

	return nullptr;
}
