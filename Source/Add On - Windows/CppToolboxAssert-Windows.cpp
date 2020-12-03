//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(const SError& error, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log error
	CLogServices::logError(error, CString::mEmpty, file, proc, line);

	// Log stack trace
	CLogServices::logMessage(CString(OSSTR("Unable to retrieve backtrace symbols")));

#if defined(DEBUG)
	// Stop in debugger so we can analyze stuffs
	CString	description = error.getDescription();
	CCoreServices::stopInDebugger(error.getCode(), description.getOSString());
#else
	// Trigger the crash reporting system so we can collect info
	::abort();
#endif
}
