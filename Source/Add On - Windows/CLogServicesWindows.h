//----------------------------------------------------------------------------------------------------------------------
//	CLogServicesWindows.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLogServicesWindows

class CLogServicesWindows {
	// Methods
	public:
						// Class methods
		static	void	logSystemInfo(const CString& productAndVersion);
};
