//----------------------------------------------------------------------------------------------------------------------
//	CLogServicesWindows.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServicesWindows.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLogServicesWindows

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CLogServicesWindows::logSystemInfo(const CString& productAndVersion)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write our first message
	CLogServices::logMessage(CString(OSSTR("Product: ")) + productAndVersion);

	// Log system info
	CLogServices::logMessage(CString(OSSTR("System Info")));

	// Processor Count
	CLogServices::logMessage(
			CString(OSSTR("\tRunning ")) + CString(CCoreServices::getTotalProcessorCoresCount()) +
					CString(OSSTR(" CPUs/Cores")));

	// Physical RAM Size
	CLogServices::logMessage(
			CString(OSSTR("\tPhysical RAM: ")) +
					CString(CCoreServices::getPhysicalMemoryByteCount(),
							(CString::SpecialFormattingOptions)
									(CString::kSpecialFormattingOptionsBytesBinary |
									CString::kSpecialFormattingOptionsBytesBinaryDoEasyRead |
									CString::kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas|
									CString::kSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel)));

	// Done
	CLogServices::logMessage(CString::mEmpty);
	CLogServices::logMessage(CString::mEmpty);
}
