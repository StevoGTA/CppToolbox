//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaFileUseDefault.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaFile.h"

#include "CMediaSourceRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaFile

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
I<CWAVEMediaFile> CWAVEMediaFile::create()
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CWAVEMediaFile>(new CWAVEMediaFile());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("wav")) };
REGISTER_MEDIA_SOURCE(wave,
		SMediaSource(SMediaSource::Identity(MAKE_OSTYPE('w', 'a', 'v', 'e'), CString(OSSTR("WAVE"))),
				TSARRAY_FROM_C_ARRAY(CString, sExtensions), CWAVEMediaFile::import));
