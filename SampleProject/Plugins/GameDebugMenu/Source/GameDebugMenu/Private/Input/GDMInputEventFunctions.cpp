/**
* Copyright (c) 2024 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMInputEventFunctions.h"

FName UGDMInputEventFunctions::GetGDMInputEventName_Up()
{
	return GDMInputEventNames::Up;
}

FName UGDMInputEventFunctions::GetGDMInputEventName_Down()
{
	return GDMInputEventNames::Down;
}

FName UGDMInputEventFunctions::GetGDMInputEventName_Left()
{
	return GDMInputEventNames::Left;
}

FName UGDMInputEventFunctions::GetGDMInputEventName_Right()
{
	return GDMInputEventNames::Right;
}

FName UGDMInputEventFunctions::GetGDMInputEventName_Decide()
{
	return GDMInputEventNames::Decide;
}

FName UGDMInputEventFunctions::GetGDMInputEventName_Cancel()
{
	return GDMInputEventNames::Cancel;
}
