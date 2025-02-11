/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Misc/OutputDevice.h"
#include <Logging/LogVerbosity.h>

class AGameDebugMenuManager;

/**
 * DebugMenuで使用できるようにするOutputLogの文字情報
 * プロジェクト名.log取得したかったけど起動中はアクセスできないため文字列情報はこれから取得。
 * ただAGameDebugMenuManagerが生成されてから動作するので正確には同じではない
 */
class GAMEDEBUGMENU_API FGDMOutputDevice : public FOutputDevice
{
	TArray<FString> Logs;
	TArray<FString> CommandHistory;
	mutable FCriticalSection CommandHistoryMutex;
	
public:
	FGDMOutputDevice();
	virtual ~FGDMOutputDevice() override;
	virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const class FName& Category, const double Time) override;
	virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const class FName& Category) override;

public:
	TArray<FString> GetLogs() const;
	TArray<FString> GetCommandHistory() const;
	void ClearCommandHistory();
};
