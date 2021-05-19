//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMPEG4MediaSource

class CMPEG4MediaSourceInternals;
class CMPEG4MediaSource : public CAtomMediaSource {
	// Methods
	public:
							// Lifecycle methods
							CMPEG4MediaSource(const I<CDataSource>& dataSource);
							~CMPEG4MediaSource();

							// CMediaSource methods
		OI<SError>			loadTracks();
		TArray<CAudioTrack>	getAudioTracks();
		TArray<CVideoTrack>	getVideoTracks();

	// Properties
	private:
		CMPEG4MediaSourceInternals*	mInternals;
};
