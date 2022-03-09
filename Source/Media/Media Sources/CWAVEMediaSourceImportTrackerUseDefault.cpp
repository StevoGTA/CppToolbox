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
