//----------------------------------------------------------------------------------------------------------------------
//	CMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CAudioTrack.h"
#include "CByteParceller.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaSource

class CMediaSource {
	// Methods
	public:
									// Lifecycle methods
									CMediaSource() {}
		virtual						~CMediaSource() {}

									// Instance methods
		virtual	OI<SError>			loadTracks(const CByteParceller& byteParceller) = 0;
		virtual	TArray<CAudioTrack>	getAudioTracks() { return TNArray<CAudioTrack>(); }
};
