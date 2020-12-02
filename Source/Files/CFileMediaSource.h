//----------------------------------------------------------------------------------------------------------------------
//	CFileMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  CFileMediaSource

class CFileMediaSource : public CMediaSource {
	// Methods
	public:
						// Lifecycle methods
						CFileMediaSource(const CFile& file) {}
						~CFileMediaSource() {}

						// Instance methods
};
