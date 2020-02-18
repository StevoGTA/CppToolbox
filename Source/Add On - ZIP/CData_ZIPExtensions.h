//----------------------------------------------------------------------------------------------------------------------
//	CData_ZIPExtensions.h			©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CData_ZIPExtensions

class CData_ZIPExtensions {
	// Methods
	public:
						// Class methods
		static	CData	uncompressDataAsZIP(const CData& data, OV<CDataSize> uncompressedDataSize = OV<CDataSize>());
};
