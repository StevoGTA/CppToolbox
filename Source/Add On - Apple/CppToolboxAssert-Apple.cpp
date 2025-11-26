//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

#include <execinfo.h>

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(const SError& error, const char* file, UInt32 fileByteCount, const char* func,
		UInt32 funcByteCount, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log error
	CLogServices::logError(error, CString::mEmpty, CString(file, fileByteCount, CString::kEncodingUTF8),
			CString(func, funcByteCount, CString::kEncodingUTF8), line);

	// Log stack trace
	void*	array[256];
	int		size = ::backtrace(array, sizeof(array) / sizeof(void*));
	char**	symbols = ::backtrace_symbols(array, size);
	if (symbols != nil) {
		// Log symbols
		for (int i = 0; i < size; i++)
			// Log this symbol
			CLogServices::logMessage(CString(symbols[i], 1024, CString::kEncodingUTF8));
		::free(symbols);
	} else
		// Unable to log symbols
		CLogServices::logMessage(CString(OSSTR("Unable to retrieve backtrace symbols")));

#if defined(DEBUG)
	// Stop in debugger so we can analyze stuffs
	CCoreServices::stopInDebugger();
#else
	// Trigger the crash reporting system so we can collect info
	::kill(::getpid(), SIGABRT);
#endif
}
