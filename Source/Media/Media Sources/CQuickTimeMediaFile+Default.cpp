//----------------------------------------------------------------------------------------------------------------------
//	CQuickTimeMediaFile+Default.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CQuickTimeMediaFile.h"

#include "CMediaSourceRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static I<SMediaSource::ImportResult> sImport(const SMediaSource::ImportSetup& importSetup)
//----------------------------------------------------------------------------------------------------------------------
{
	return CQuickTimeMediaFile().import(importSetup);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("mov")) };
REGISTER_MEDIA_SOURCE(quicktime,
		SMediaSource(SMediaSource::Identity(CQuickTimeMediaFile::mID, CString(OSSTR("QuickTime"))),
				TSARRAY_FROM_C_ARRAY(CString, sExtensions), sImport));
