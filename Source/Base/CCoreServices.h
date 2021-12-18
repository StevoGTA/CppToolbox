//----------------------------------------------------------------------------------------------------------------------
//	CCoreServices.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SVersionInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreServices

class CCoreServices {
	// Methods
	public:
											// Info methods
		static			UInt32				getTotalProcessorCoresCount();
		static			UInt64				getPhysicalMemoryByteCount();

#if defined(TARGET_OS_MACOS)
		static	const	SSystemVersionInfo&	getSystemVersion();
		static	const	SVersionInfo&		getCoreAudioVersion();
		static	const	CString&			getProcessorInfo();
		static			UInt32				getPhysicalMemoryPageSize();
#endif

											// Debugger methods
		static			void				stopInDebugger(SInt32 code = 0, OSStringVar(message) = OSSTR(""));
};
