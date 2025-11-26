//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreServices.h"

#include "SError.h"

#ifdef __cplusplus_winrt
	// C++/CX
	#include "CPlatform.h"

	using namespace Platform;
	using namespace Windows::ApplicationModel;
#else
	// C++/WinRT
	#include <winrt/Windows.System.Diagnostics.h>

	using namespace winrt::Windows::System::Diagnostics;
#endif

#undef Delete
#include <Windows.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

#ifdef __cplusplus_winrt
	// C++/CX
	static	UInt32	sCountSetBits(ULONG_PTR bits);
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreServices methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreServices::getTotalProcessorCoresCount()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	UInt32	sTotalProcessorCoresCount = 0;
	if (sTotalProcessorCoresCount == 0) {
		//// Create buffer
		//SYSTEM_LOGICAL_PROCESSOR_INFORMATION*	buffer = NULL;
		//DWORD									size = 0;
		//GetLogicalProcessorInformation(buffer, &size);
		//buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*) malloc(size);

		//// Get info
		//if (GetLogicalProcessorInformation(buffer, &size) == 1) {
		//	// Success
		//	int										infoCount = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		//	SYSTEM_LOGICAL_PROCESSOR_INFORMATION*	systemLogicalProcessorInformation = buffer;
		//	for (int i = 0; i < infoCount; i++, systemLogicalProcessorInformation++) {
		//		// Check relationship
		//		if (systemLogicalProcessorInformation->Relationship == RelationProcessorCore) {
		//			// Processor core
		//			sTotalProcessorCoresCount += sCountSetBits(systemLogicalProcessorInformation->ProcessorMask);
		//		}
		//	}
		//}

		//// Cleanup
		//free(buffer);

		// Get sytem info
		SYSTEM_INFO	systemInfo;
		::GetSystemInfo(&systemInfo);

		// Store info
		sTotalProcessorCoresCount = systemInfo.dwNumberOfProcessors;
	}


	return sTotalProcessorCoresCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CCoreServices::getPhysicalMemoryByteCount()
//----------------------------------------------------------------------------------------------------------------------
{
	static	int64_t	sPhysicalMemoryByteCount = 0;

	if (sPhysicalMemoryByteCount == 0)
		// Get info
#ifdef __cplusplus_winrt
		(void) sPhysicalMemoryByteCount;
#else
		sPhysicalMemoryByteCount =
				SystemDiagnosticInfo::GetForCurrentSystem().MemoryUsage().GetReport().TotalPhysicalSizeInBytes();
#endif

	return sPhysicalMemoryByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreServices::stopInDebugger(SInt32 code, OSStringVar(message))
//----------------------------------------------------------------------------------------------------------------------
{
#ifdef __cplusplus_winrt
	// C++/CX
	throw Platform::Exception::CreateException(code, ref new String(message));
#else
	// C++/WinRT
	_ASSERT_EXPR(true, message);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

#ifdef __cplusplus_winrt
// C++/CX
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
#endif
