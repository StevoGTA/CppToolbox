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
		static	const	CString&		getCopyright();
};
