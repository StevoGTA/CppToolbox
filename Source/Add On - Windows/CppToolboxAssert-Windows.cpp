//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(const SError& error, const char* file, UInt32 fileByteCount, const char* func,
		UInt32 funcByteCount, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log error
	CLogServices::logError(error, CString::mEmpty, CString(file, fileByteCount, CString::kEncodingUTF8),
			CString(func, funcByteCount, CString::kEncodingUTF8), line);

	// Log stack trace
	CLogServices::logMessage(CString(OSSTR("Unable to retrieve backtrace symbols")));

#if defined(DEBUG)
	// Stop in debugger so we can analyze stuffs
	CString	description = error.getDefaultLocalizedDescription();
	CCoreServices::stopInDebugger(error.getCode(), description.getOSString());
#else
	// Trigger the crash reporting system so we can collect info
	::abort();
#endif
}
