//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreServices.h"

using namespace Platform;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreServices methods

//----------------------------------------------------------------------------------------------------------------------
const SSystemVersionInfo& CCoreServices::getSystemVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	static	SSystemVersionInfo*	sVersionInfo = nil;

	if (sVersionInfo == nil) {
		// Get info
		AssertFailUnimplemented();
	}

	return *sVersionInfo;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getTotalProcessorCoresCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int	sTotalProcessorCoresCount = 0;

	if (sTotalProcessorCoresCount == 0) {
		// Get info
		AssertFailUnimplemented();
	}

	return sTotalProcessorCoresCount;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CCoreServices::getProcessorInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CString*	sProcessorInfoString = nil;

	if (sProcessorInfoString == nil) {
		// Get info
		AssertFailUnimplemented();
	}

    return (sProcessorInfoString != nil) ? *sProcessorInfoString : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CCoreServices::getPhysicalMemoryByteCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int64_t	sPhysicalMemoryByteCount = 0;

	if (sPhysicalMemoryByteCount == 0) {
		// Get info
		AssertFailUnimplemented();
	}

	return sPhysicalMemoryByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getPhysicalMemoryPageSize()
//----------------------------------------------------------------------------------------------------------------------
{
	static	UInt32	sPhysicalMemoryPageSize = 0;

		AssertFailUnimplemented();

	return sPhysicalMemoryPageSize;
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreServices::stopInDebugger(SInt32 code, OSStringVar(message))
//----------------------------------------------------------------------------------------------------------------------
{
	throw Platform::Exception::CreateException(code, ref new String(message));
}
