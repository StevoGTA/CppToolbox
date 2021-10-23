//----------------------------------------------------------------------------------------------------------------------
//	SError-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError-Apple.h"

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromOSStatus(OSStatus status)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	char*	string = ::GetMacOSStatusCommentString(status);

	return SError(CString(OSSTR("OSStatus")), status, (string != nil) ? CString(string) : CString(status));
}
#endif
