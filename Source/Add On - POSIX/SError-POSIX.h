//----------------------------------------------------------------------------------------------------------------------
//	SError-POSIX.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#define SErrorFromPOSIXerror(e)	SError(CString(OSSTR("POSIX")), e, CString(e))
