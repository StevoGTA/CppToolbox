//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreServices.h"

#include "CPlatform.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

using namespace Platform;
using namespace Windows::ApplicationModel;

 //----------------------------------------------------------------------------------------------------------------------
 // MARK: Local proc declarations

 static	UInt32	sCountSetBits(ULONG_PTR bits);

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
		Package^		package = Package::Current;
		PackageId^		packageId = package->Id;
		String^			displayName = package->DisplayName;
		PackageVersion	packageVersion = packageId->Version;

		sVersionInfo =
				new SSystemVersionInfo(CPlatform::stringFrom(displayName), packageVersion.Major, packageVersion.Minor,
						packageVersion.Revision, packageVersion.Build);
	}

	return *sVersionInfo;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getTotalProcessorCoresCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	UInt32	sTotalProcessorCoresCount = 0;

	if (sTotalProcessorCoresCount == 0) {
		// Create buffer
		SYSTEM_LOGICAL_PROCESSOR_INFORMATION*	buffer = NULL;
		DWORD									size = 0;
		GetLogicalProcessorInformation(buffer, &size);
		buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*) malloc(size);

		// Get info
		if (GetLogicalProcessorInformation(buffer, &size) == 1) {
			// Success
			int										infoCount = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			SYSTEM_LOGICAL_PROCESSOR_INFORMATION*	systemLogicalProcessorInformation = buffer;
			for (int i = 0; i < infoCount; i++, systemLogicalProcessorInformation++) {
				// Check relationship
				if (systemLogicalProcessorInformation->Relationship == RelationProcessorCore) {
					// Processor core
					sTotalProcessorCoresCount += sCountSetBits(systemLogicalProcessorInformation->ProcessorMask);
				}
			}
		}

		// Cleanup
		free(buffer);
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

 //----------------------------------------------------------------------------------------------------------------------
 //----------------------------------------------------------------------------------------------------------------------
 // MARK: - Local proc definitions

 //----------------------------------------------------------------------------------------------------------------------
UInt32 sCountSetBits(ULONG_PTR bits)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DWORD		bitCount = sizeof(ULONG_PTR) * 8 - 1;
	UInt32		bitsSetCount = 0;
	ULONG_PTR	bitTest = (ULONG_PTR) 1 << bitCount;

	// Iterate bits
	for (DWORD i = 0; i <= bitCount; i++) {
		// Check
		bitsSetCount += (bits & bitTest) ? 1 : 0;
		bitTest >>= 1;
	}

	return bitsSetCount;
}
