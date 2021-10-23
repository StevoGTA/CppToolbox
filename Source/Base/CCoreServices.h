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
		static	const	SSystemVersionInfo&	getSystemVersion();
		static			UInt32				getTotalProcessorCoresCount();
		static	const	CString&			getProcessorInfo();
		static			UInt64				getPhysicalMemoryByteCount();
		static			UInt32				getPhysicalMemoryPageSize();

#if defined(TARGET_OS_MACOS)
		static	const	SVersionInfo&		getCoreAudioVersion();
#endif

											// Debugger methods
		static			void				stopInDebugger(SInt32 code = 0, OSStringVar(message) = OSSTR(""));
};
