//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFileUseDefault.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMPEG4MediaFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static I<SMediaSource::ImportResult> sImport(const SMediaSource::ImportSetup& importSetup)
//----------------------------------------------------------------------------------------------------------------------
{
	return CMPEG4MediaFile().import(importSetup);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("m4a")), CString(OSSTR("m4v")), CString(OSSTR("mp4")) };
REGISTER_MEDIA_SOURCE(mp4,
		SMediaSource(SMediaSource::Identity(CMPEG4MediaFile::mID, CString(OSSTR("MPEG 4"))),
				TSARRAY_FROM_C_ARRAY(CString, sExtensions), sImport));
