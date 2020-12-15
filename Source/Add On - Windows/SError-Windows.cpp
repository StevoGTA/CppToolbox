//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError-Windows.h"

#include "comdef.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromHRESULT(HRESULT result)
//----------------------------------------------------------------------------------------------------------------------
{
	return SError(CString(OSSTR("HRESULT")), result, CString(_com_error(result, nullptr).ErrorMessage()));
}

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromWindowsError(DWORD error)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert code to string
	wchar_t	buffer[1024];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, (sizeof(buffer) / sizeof(wchar_t)), NULL);

	return SError(CString(OSSTR("Windows")), error, CString(buffer));
}
