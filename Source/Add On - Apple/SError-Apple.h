//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"
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

#define LogOSStatusIfFailed(status, method)												\
				if (status != noErr)													\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									SErrorFromOSStatus(status).getDefaultDescription());
#define LogOSStatusIfFailedAndReturn(status, method)										\
				if (status != noErr) {														\
					CLogServices::logError(													\
							CString(method) + CString(OSSTR(" returned ")) +				\
									SErrorFromOSStatus(status).getDefaultDescription());	\
					return;																	\
				}
#define LogOSStatusIfFailedAndReturnValue(status, method, value)							\
				if (status != noErr) {														\
					CLogServices::logError(													\
							CString(method) + CString(OSSTR(" returned ")) +				\
									SErrorFromOSStatus(status).getDefaultDescription());	\
					return value;															\
				}

#define	ReturnErrorIfFailed(status, method)											\
				if (status != noErr) {												\
					SError	_error = SErrorFromOSStatus(status);					\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									_error.getDefaultDescription());				\
																					\
					return OV<SError>(_error);										\
				}
#define	ReturnValueIfFailed(status, method, value)									\
				if (status != noErr) {												\
					SError	_error = SErrorFromOSStatus(status);					\
					CLogServices::logError(											\
							CString(method) + CString(OSSTR(" returned ")) +		\
									_error.getDefaultDescription());				\
																					\
					return value;													\
				}
