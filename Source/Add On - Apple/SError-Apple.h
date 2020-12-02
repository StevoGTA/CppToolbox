//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#define SErrorFromNSError(e)												\
		SError(CString((__bridge CFStringRef) e.domain), (SInt32) e.code,	\
				CString((__bridge CFStringRef) e.localizedDescription))
