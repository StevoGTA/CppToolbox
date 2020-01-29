//----------------------------------------------------------------------------------------------------------------------
//  CppToolboxAssert.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
void eAssertHandleProc(UError error, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	CLogServices::logError(error, CString::mEmpty, file, proc, line);
#if defined(DEBUG)
	CCoreServices::stopInDebugger();
#else
	exit(EXIT_FAILURE);
#endif
}
