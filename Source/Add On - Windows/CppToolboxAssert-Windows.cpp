//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(UError error, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log error
	CLogServices::logError(error, CString::mEmpty, file, proc, line);

	// Log stack trace
	CLogServices::logMessage(CString(OSSTR("Unable to retrieve backtrace symbols")));

#if defined(DEBUG)
	// Stop in debugger so we can analyze stuffs
	CCoreServices::stopInDebugger(GET_UErrorError(error));
#else
	// Trigger the crash reporting system so we can collect info
	::abort();
#endif
}
