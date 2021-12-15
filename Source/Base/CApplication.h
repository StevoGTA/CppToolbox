//----------------------------------------------------------------------------------------------------------------------
//	CApplication.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SVersionInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication

class CApplication {
	// Methods
	public:
										// Class methods
		static	const	CString&		getProductName();
		static	const	SVersionInfo&	getVersion();
		static	const	CString&		getProductNameAndVersion();
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		static	const	CString&		getCopyright();
#endif
};
