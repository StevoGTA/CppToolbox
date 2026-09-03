//----------------------------------------------------------------------------------------------------------------------
//	SError-POSIX.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#include <cstring>

#define SErrorFromPOSIXerror(e)	\
		SError(CString(OSSTR("POSIX")), e, CString(::strerror(e), ::strlen(::strerror(e)), CString::kEncodingUTF8))
