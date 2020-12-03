//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

SError SErrorFromWindowsError(DWORD error);
