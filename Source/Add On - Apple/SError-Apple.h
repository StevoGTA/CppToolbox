//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"
#include "SError.h"

extern	SError	SErrorFromCFError(CFErrorRef errorRef);

#if defined(TARGET_OS_MACOS)
	extern	SError	SErrorFromOSStatus(OSStatus status);
#else
	#define SErrorFromOSStatus(status)	SError(CString(OSSTR("OSStatus")), status, CString(status))
#endif

#define SErrorFromNSError(e)												\
		SError(CString((__bridge CFStringRef) e.domain), (SInt32) e.code,	\
				CString((__bridge CFStringRef) e.localizedDescription))

#define LogOSStatusIfFailed(status, method)															\
				if (status != noErr)																\
					CLogServices::logError(															\
							method + CString(OSSTR(" returned ")) +									\
									SErrorFromOSStatus(status).getDefaultLocalizedDescription());
#define LogOSStatusIfFailedAndReturn(status, method)												\
				if (status != noErr) {																\
					CLogServices::logError(															\
							method + CString(OSSTR(" returned ")) +									\
									SErrorFromOSStatus(status).getDefaultLocalizedDescription());	\
					return;																			\
				}
#define LogOSStatusIfFailedAndReturnValue(status, method, value)									\
				if (status != noErr) {																\
					CLogServices::logError(															\
							method + CString(OSSTR(" returned ")) +									\
									SErrorFromOSStatus(status).getDefaultLocalizedDescription());	\
					return value;																	\
				}

#define	ReturnErrorIfFailed(status, method)											\
				if (status != noErr) {												\
					SError	_error = SErrorFromOSStatus(status);					\
					CLogServices::logError(											\
							method + CString(OSSTR(" returned ")) +					\
									_error.getDefaultLocalizedDescription());		\
																					\
					return OV<SError>(_error);										\
				}
#define	ReturnValueIfFailed(status, method, value)									\
				if (status != noErr) {												\
					SError	_error = SErrorFromOSStatus(status);					\
					CLogServices::logError(											\
							method + CString(OSSTR(" returned ")) +					\
									_error.getDefaultLocalizedDescription());		\
																					\
					return value;													\
				}
