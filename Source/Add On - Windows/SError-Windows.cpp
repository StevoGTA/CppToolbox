//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromWindowsError(DWORD error)
//----------------------------------------------------------------------------------------------------------------------
{
	wchar_t	buffer[1024];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, (sizeof(buffer) / sizeof(wchar_t)), NULL);

	return SError(CString(OSSTR("Windows")), error, CString(buffer));
}
