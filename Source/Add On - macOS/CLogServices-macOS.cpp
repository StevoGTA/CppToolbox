//----------------------------------------------------------------------------------------------------------------------
//	CLogServices-macOS.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices-macOS.h"

#include "CCoreServices.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLogServicesMacOS

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CLogServicesMacOS::logSystemInfo(const CString& productAndVersion)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write our first message
	CLogServices::logMessage(CString(OSSTR("Product: ")) + productAndVersion);

	// Log system info
	CLogServices::logMessage(CString(OSSTR("System Info")));

	// OS Version
	CLogServices::logMessage(CString(OSSTR("\tSystem Version: ")) + CCoreServices::getSystemVersion().getString());

	// Processor Count
	CLogServices::logMessage(
			CString(OSSTR("\tRunning ")) + CString(CCoreServices::getTotalProcessorCoresCount()) +
					CString(OSSTR(" CPUs/Cores - ")) + CCoreServices::getProcessorInfo());

	// Physical RAM Size
	CLogServices::logMessage(
			CString(OSSTR("\tPhysical RAM: ")) +
					CString(CCoreServices::getPhysicalMemoryByteCount(),
							(EStringSpecialFormattingOptions) (kStringSpecialFormattingOptionsBytesBinary |
									kStringSpecialFormattingOptionsBytesBinaryDoEasyRead |
									kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas|
									kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel)));

	// Core Audio Version
	CLogServices::logMessage(
			CString(OSSTR("\tCore Audio Version: ")) + CCoreServices::getCoreAudioVersion().getString());

	// Done
	CLogServices::logMessage(CString::mEmpty);
	CLogServices::logMessage(CString::mEmpty);
}
