//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows-WinRT.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#include <winrt/base.h>

using namespace winrt;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

SError	SErrorFromHRESULTError(const hresult_error& hresultError);
