/**
* Copyright (c) 2023 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#include "Input/GDMInputInterface.h"

bool IGDMInputInterface::IsEnableInputGDM_Implementation()
{
	return true;
}

int32 IGDMInputInterface::GetInputPriorityGDM_Implementation()
{
	/** Blueprint関数の「GDM Input Priority First」と同じ */
	return 10;
}

