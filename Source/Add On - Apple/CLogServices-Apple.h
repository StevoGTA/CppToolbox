//----------------------------------------------------------------------------------------------------------------------
//	CLogServices-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"
#include "SError-Apple.h"

#define LogOSStatusIfFailed(status, method)											\
				if (status != noErr)												\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									SErrorFromOSStatus(status).getDescription());
#define LogOSStatusIfFailedAndReturn(status, method)								\
				if (status != noErr) {												\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									SErrorFromOSStatus(status).getDescription());	\
					return;															\
				}
#define LogOSStatusIfFailedAndReturnValue(status, method, value)					\
				if (status != noErr) {												\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									SErrorFromOSStatus(status).getDescription());	\
					return value;													\
				}
