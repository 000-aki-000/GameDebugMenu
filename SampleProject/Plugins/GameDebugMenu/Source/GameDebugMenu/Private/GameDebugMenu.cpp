/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "GameDebugMenu.h"

#define LOCTEXT_NAMESPACE "FGameDebugMenuModule"

void FGameDebugMenuModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("FGameDebugMenuModule StartupModule"));
}

void FGameDebugMenuModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("FGameDebugMenuModule ShutdownModule"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameDebugMenuModule, GameDebugMenu)