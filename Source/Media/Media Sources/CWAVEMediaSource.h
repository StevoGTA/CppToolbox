//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSource

class CWAVEMediaSourceInternals;
class CWAVEMediaSource : public CMediaSource {
	// Methods
	public:
							// Lifecycle methods
							CWAVEMediaSource();
							~CWAVEMediaSource();

							// CMediaSource methods
		OI<SError>			loadTracks(const CByteParceller& byteParceller);
		TArray<CAudioTrack>	getAudioTracks();

	// Properties
	private:
		CWAVEMediaSourceInternals*	mInternals;
};
