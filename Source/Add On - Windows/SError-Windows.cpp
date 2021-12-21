//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError-Windows.h"

#undef Delete
#include "comdef.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromHRESULT(HRESULT result)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	return SError(CString(OSSTR("HRESULT")), result, CString(_com_error(result, nullptr).ErrorMessage()));
}

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromWindowsError(DWORD error)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert code to string
	TCHAR	buffer[1024];
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, (sizeof(buffer) / sizeof(TCHAR)), NULL);

	return SError(CString(OSSTR("Windows")), error, CString(buffer).replacingSubStrings(CString(OSSTR("\r\n"))));
}

//----------------------------------------------------------------------------------------------------------------------
SError SErrorFromWindowsGetLastError()
//----------------------------------------------------------------------------------------------------------------------
{
	// Return error
	return SErrorFromWindowsError(::GetLastError());
}
