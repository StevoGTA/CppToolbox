//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

#include <execinfo.h>

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(UError error, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log error
	CLogServices::logError(error, CString::mEmpty, file, proc, line);

	// Log stack trace
	void*	array[256];
	int		size = backtrace(array, sizeof(array) / sizeof(void*));
	char**	symbols = backtrace_symbols(array, size);
	if (symbols != nil) {
		// Log symbols
		for (int i = 0; i < size; i++)
			CLogServices::logMessage(CString(symbols[i]));
		free(symbols);
	} else
		// Unable to log symbols
		CLogServices::logMessage(CString(OSSTR("Unable to retrieve backtrace symbols")));

#if defined(DEBUG)
	// Stop in debugger so we can analyze stuffs
	CCoreServices::stopInDebugger();
#else
	// Trigger the crash reporting system so we can collect info
	kill(getpid(), SIGABRT);
#endif
}
