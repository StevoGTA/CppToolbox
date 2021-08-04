//----------------------------------------------------------------------------------------------------------------------
//	SMediaTracks.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CVideoTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaTracks

struct SMediaTracks {

									// Lifecycle methods
									SMediaTracks(const TArray<CAudioTrack>& audioTracks,
											const TArray<CVideoTrack>& videoTracks) :
										mAudioTracks(audioTracks), mVideoTracks(videoTracks)
										{}
									SMediaTracks(const TArray<CAudioTrack>& audioTracks) : mAudioTracks(audioTracks) {}
									SMediaTracks(const TArray<CVideoTrack>& videoTracks) : mVideoTracks(videoTracks) {}
									SMediaTracks(const SMediaTracks& other) :
										mAudioTracks(other.mAudioTracks), mVideoTracks(other.mVideoTracks)
										{}

									// Instance methods
	const	TArray<CAudioTrack>&	getAudioTracks() const
										{ return mAudioTracks; }
	const	TArray<CVideoTrack>&	getVideoTracks() const
										{ return mVideoTracks; }

	// Properties
	private:
		TNArray<CAudioTrack>	mAudioTracks;
		TNArray<CVideoTrack>	mVideoTracks;
};
