//----------------------------------------------------------------------------------------------------------------------
//	CLogServices-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"

#define LOG_OSSTATUS_IF_FAILED(status, method)										\
				if (status != noErr)												\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									SErrorFromOSStatus(status).getDescription());
