//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSourceImportTrackerUseDefault.cpp			©2022 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaSourceImportTracker

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
I<CWAVEMediaSourceImportTracker> CWAVEMediaSourceImportTracker::instantiate()
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CWAVEMediaSourceImportTracker>(new CDefaultWAVEMediaSourceImportTracker());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("wav")) };

REGISTER_MEDIA_SOURCE(wave,
		SMediaSource(MAKE_OSTYPE('w', 'a', 'v', 'e'), CString(OSSTR("WAVE")), TSArray<CString>(sExtensions, 1),
				CWAVEMediaSource::queryTracks));
