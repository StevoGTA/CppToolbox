//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#define SErrorFromNSError(e)												\
		SError(CString((__bridge CFStringRef) e.domain), (SInt32) e.code,	\
				CString((__bridge CFStringRef) e.localizedDescription))

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
#define SErrorFromOSStatus(status)	SError(CString(OSSTR("OSStatus")), status, CString(status))
#endif

#if TARGET_OS_MACOS
#define SErrorFromOSStatus(status)															\
		SError(CString(OSSTR("OSStatus")), status,											\
				(::GetMacOSStatusCommentString(status) != nil) ?							\
						CString(::GetMacOSStatusCommentString(status)) : CString(status))
#endif
