//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSource

class CWAVEMediaSourceInternals;
class CWAVEMediaSource : public CChunkMediaSource {
	// Methods
	public:
							// Lifecycle methods
							CWAVEMediaSource(const I<CDataSource>& dataSource);
							~CWAVEMediaSource();

							// CMediaSource methods
		OI<SError>			loadTracks();
		TArray<CAudioTrack>	getAudioTracks();

	// Properties
	private:
		CWAVEMediaSourceInternals*	mInternals;
};
