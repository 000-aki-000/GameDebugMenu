/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GDMInputEventFunctions.generated.h"

/**
* DebugMenuでの入力イベント名
*/
namespace GDMInputEventNames
{
	static FName Up(TEXT("GameDebugMenu_Up"));
	static FName Down(TEXT("GameDebugMenu_Down"));
	static FName Left(TEXT("GameDebugMenu_Left"));
	static FName Right(TEXT("GameDebugMenu_Right"));
	static FName Decide(TEXT("GameDebugMenu_Decide"));
	static FName Cancel(TEXT("GameDebugMenu_Cancel"));
	static FName MenuOpenAndClose(TEXT("GameDebugMenu_MenuOpenAndClose"));
	static FName DebugReport(TEXT("GameDebugMenu_DebugReport"));

	/* DebugCameraControllerのイベント */
	static FName OrbitHitPoint(TEXT("DebugCamera_OrbitHitPoint"));

	static FName AxisMoveForward(TEXT("GameDebugMenu_MoveForward"));
	static FName AxisMoveRight(TEXT("GameDebugMenu_MoveRight"));
	static FName AxisLookRight(TEXT("GameDebugMenu_LookRight"));
	static FName AxisLookUp(TEXT("GameDebugMenu_LookUp"));
};


UCLASS()
class GAMEDEBUGMENU_API UGDMInputEventFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Up"))
	static FName GetGDMInputEventName_Up();																			 
																													 
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Down"))
	static FName GetGDMInputEventName_Down();																		 
																													 
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Left"))
	static FName GetGDMInputEventName_Left();																		 
																													 
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Right"))
	static FName GetGDMInputEventName_Right();																		 
																													 
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Decide"))
	static FName GetGDMInputEventName_Decide();																		 
																													 
	UFUNCTION(BlueprintPure, Category = "GDM|Functions", meta = (Keywords = "DebugMenu GDM", CompactNodeTitle = "GDM_InputEventName_Cancel"))
	static FName GetGDMInputEventName_Cancel();
};
