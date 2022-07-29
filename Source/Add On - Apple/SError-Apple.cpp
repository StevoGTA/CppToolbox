//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError-Apple.h"

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromOSStatus(OSStatus status)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check range
	if (status < 0) {
		// Negative integer
		const	char*	string = ::GetMacOSStatusCommentString(status);

		return SError(CString(OSSTR("OSStatus")), status, (string != nil) ? CString(string) : CString(status));
	} else {
		// OSType
		return SError(CString(OSSTR("OSStatus")), status, CString((UInt32) status, true, false));
	}
}
#endif
