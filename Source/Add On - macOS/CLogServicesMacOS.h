//----------------------------------------------------------------------------------------------------------------------
//	CLogServicesMacOS.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServicesMacOS

class CLogServicesMacOS {
	// Methods
	public:
						// Class methods
		static	void	logSystemInfo(const CString& productAndVersion);
};
