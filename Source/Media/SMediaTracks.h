//----------------------------------------------------------------------------------------------------------------------
//	SMediaTracks.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CVideoTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMediaTrackInfo

template <typename T> struct TMediaTrackInfo {
				// Lifecycle methods
				TMediaTrackInfo<T>(const T& mediaTrack, const I<CCodec::DecodeInfo>& decodeInfo) :
					mMediaTrack(mediaTrack), mDecodeInfo(OI<I<CCodec::DecodeInfo> >(decodeInfo))
					{}
				TMediaTrackInfo<T>(const T& mediaTrack) : mMediaTrack(mediaTrack) {}
				TMediaTrackInfo<T>(const TMediaTrackInfo<T>& other) :
					mMediaTrack(other.mMediaTrack), mDecodeInfo(other.mDecodeInfo)
					{}

				// Class methods
	static	T	getMediaTrack(TMediaTrackInfo<T>* mediaTrackInfo)
					{ return mediaTrackInfo->mMediaTrack; }

	// Properties
	T							mMediaTrack;
	OI<I<CCodec::DecodeInfo> >	mDecodeInfo;
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
		typedef	TMediaTrackInfo<CAudioTrack>	AudioTrackInfo;
		typedef	TMediaTrackInfo<CVideoTrack>	VideoTrackInfo;

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
															(TNArray<CAudioTrack>::MappingProc)
																	AudioTrackInfo::getMediaTrack),
													TNArray<CVideoTrack>(mVideoTrackInfos,
															(TNArray<CVideoTrack>::MappingProc)
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
