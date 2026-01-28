//----------------------------------------------------------------------------------------------------------------------
//	CApplication.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#if defined(TARGET_OS_WINDOWS)
	#include "CFolder.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication

class CApplication {
	// Methods
	public:
										// Class methods
		static	const	CString&		getProductName();
		static	const	CString&		getVersion();
		static	const	CString&		getProductNameAndVersion();
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		static	const	CString&		getCopyright();
#endif

#if defined(TARGET_OS_WINDOWS)
		static	const	CFolder&		getFolder();
#endif
};
