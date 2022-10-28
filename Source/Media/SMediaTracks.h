//----------------------------------------------------------------------------------------------------------------------
//	SMediaTracks.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CVideoTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMediaTrackInfo

template <typename T, typename U> struct TMediaTrackInfo {
				// Lifecycle methods
				TMediaTrackInfo<T, U>(const T& mediaTrack, const OV<I<U> >& codec) :
					mMediaTrack(mediaTrack), mCodec(codec)
					{}
				TMediaTrackInfo<T, U>(const T& mediaTrack) : mMediaTrack(mediaTrack) {}
				TMediaTrackInfo<T, U>(const TMediaTrackInfo<T, U>& other) :
					mMediaTrack(other.mMediaTrack), mCodec(other.mCodec)
					{}

				// Class methods
	static	T	getMediaTrack(TMediaTrackInfo<T, U>* mediaTrackInfo)
					{ return mediaTrackInfo->mMediaTrack; }

	// Properties
	T			mMediaTrack;
	OV<I<U> >	mCodec;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaTracks

struct SMediaTracks {

	// Lifecycle methods
	SMediaTracks(const TArray<CAudioTrack>& audioTracks, const TArray<CVideoTrack>& videoTracks) :
		mAudioTracks(audioTracks), mVideoTracks(videoTracks)
		{}
	SMediaTracks(const SMediaTracks& other) : mAudioTracks(other.mAudioTracks), mVideoTracks(other.mVideoTracks) {}

	// Properties
	TNArray<CAudioTrack>	mAudioTracks;
	TNArray<CVideoTrack>	mVideoTracks;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaTrackInfos

class CMediaTrackInfos {
	// Types
	public:
		typedef	TMediaTrackInfo<CAudioTrack, CDecodeAudioCodec>	AudioTrackInfo;
		typedef	TMediaTrackInfo<CVideoTrack, CDecodeVideoCodec>	VideoTrackInfo;

	// Methods
	public:

									// Lifecycle methods
									CMediaTrackInfos() {}
									CMediaTrackInfos(const CMediaTrackInfos& other) :
										mAudioTrackInfos(other.mAudioTrackInfos),
												mVideoTrackInfos(other.mVideoTrackInfos)
										{}

									// Instance methods
	const	TArray<AudioTrackInfo>&	getAudioTrackInfos() const
										{ return mAudioTrackInfos; }
			void					add(const AudioTrackInfo& audioTrackInfo)
										{ mAudioTrackInfos += audioTrackInfo; }

	const	TArray<VideoTrackInfo>&	getVideoTrackInfos() const
										{ return mVideoTrackInfos; }
			void					add(const VideoTrackInfo& videoTrackInfo)
										{ mVideoTrackInfos += videoTrackInfo; }

			SMediaTracks			getMediaTracks() const
										{
											return SMediaTracks(
													TNArray<CAudioTrack>(mAudioTrackInfos,
															(TNArray<CAudioTrack>::MapProc)
																	AudioTrackInfo::getMediaTrack),
													TNArray<CVideoTrack>(mVideoTrackInfos,
															(TNArray<CVideoTrack>::MapProc)
																	VideoTrackInfo::getMediaTrack));
										}
			UniversalTimeInterval	getDuration() const
										{
											// Compose total duration
											UniversalTimeInterval	duration = 0.0;
											for (TIteratorD<AudioTrackInfo> iterator = mAudioTrackInfos.getIterator();
													iterator.hasValue(); iterator.advance())
												// Update duration
												duration =
														std::max<UniversalTimeInterval>(duration,
																iterator->mMediaTrack.getInfo().getDuration());
											for (TIteratorD<VideoTrackInfo> iterator = mVideoTrackInfos.getIterator();
													iterator.hasValue(); iterator.advance())
												// Update duration
												duration =
														std::max<UniversalTimeInterval>(duration,
																iterator->mMediaTrack.getInfo().getDuration());

											return duration;
										}

	// Properties
	private:
		TNArray<AudioTrackInfo>	mAudioTrackInfos;
		TNArray<VideoTrackInfo>	mVideoTrackInfos;
};
