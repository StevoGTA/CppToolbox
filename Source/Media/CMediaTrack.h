//----------------------------------------------------------------------------------------------------------------------
//	CMediaTrack.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SMediaPosition.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaTrack

//class CMediaTrackInternals;
class CMediaTrack {
	// Methods
	public:
				// Lifecycle methods
				CMediaTrack() {}
				CMediaTrack(const CMediaTrack& other) {}
		virtual	~CMediaTrack() {}
//
//	// Properties
//	private:
//		CMediaTrackInternals*	mInternals;
};
