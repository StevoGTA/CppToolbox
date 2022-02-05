//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#define SErrorFromNSError(e)												\
		SError(CString((__bridge CFStringRef) e.domain), (SInt32) e.code,	\
				CString((__bridge CFStringRef) e.localizedDescription))

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#define SErrorFromOSStatus(status)	SError(CString(OSSTR("OSStatus")), status, CString(status))
#endif

#if defined(TARGET_OS_MACOS)
	extern SError SErrorFromOSStatus(OSStatus status);
#endif

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

#define	ReturnErrorIfFailed(status, method)											\
				if (status != noErr) {												\
					SError	error = SErrorFromOSStatus(status);						\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									error.getDescription());						\
																					\
					return OI<SError>(error);										\
				}
#define	ReturnValueIfFailed(status, method, value)									\
				if (status != noErr) {												\
					SError	error = SErrorFromOSStatus(status);						\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									error.getDescription());						\
																					\
					return value;													\
				}
