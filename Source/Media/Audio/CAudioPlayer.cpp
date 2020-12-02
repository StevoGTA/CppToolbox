//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	SUniversalTimeIntervalPref	sPlaybackBufferDurationPref(
											OSSTR("coreAudioPlayerOutputUnitReadAheadBufferTimeSecs"), 0.25);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Properties

const	UniversalTimeInterval	CAudioPlayer::kMinBufferDuration = 0.1;
const	UniversalTimeInterval	CAudioPlayer::kMaxBufferDuration = 2.0;
const	UniversalTimeInterval	CAudioPlayer::kPreviewDuration = 0.1;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioPlayer::getPlaybackBufferDuration()
//----------------------------------------------------------------------------------------------------------------------
{
	return CPreferences::mDefault.getUniversalTimeInterval(sPlaybackBufferDurationPref);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setPlaybackBufferDuration(UniversalTimeInterval playbackBufferDuration)
//----------------------------------------------------------------------------------------------------------------------
{
	CPreferences::mDefault.set(sPlaybackBufferDurationPref, playbackBufferDuration);
}
