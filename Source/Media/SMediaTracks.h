//----------------------------------------------------------------------------------------------------------------------
//	SMediaTracks.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "CVideoCodec.h"
#include "SAudio.h"
#include "SVideo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TMediaTrackInfo

template <typename MTF, typename C> struct TMediaTrackInfo {
											// Lifecycle methods
											TMediaTrackInfo<MTF, C>(const MTF& mediaTrackFormat,
													const SMedia::SegmentInfo& mediaSegmentInfo, const I<C>& codec) :
												mMediaTrackFormat(mediaTrackFormat),
														mMediaSegmentInfo(mediaSegmentInfo), mCodec(codec)
												{}
											TMediaTrackInfo<MTF, C>(const MTF& mediaTrackFormat,
													const SMedia::SegmentInfo& mediaSegmentInfo,
													const OV<I<C> >& codec = OV<I<C> >()) :
												mMediaTrackFormat(mediaTrackFormat),
														mMediaSegmentInfo(mediaSegmentInfo), mCodec(codec)
												{}
											TMediaTrackInfo<MTF, C>(const TMediaTrackInfo<MTF, C>& other) :
												mMediaTrackFormat(other.mMediaTrackFormat),
														mMediaSegmentInfo(other.mMediaSegmentInfo), mCodec(other.mCodec)
												{}

											// Instance methods
		const	MTF&						getMediaTrackFormat() const
												{ return mMediaTrackFormat; }
		const	OV<SMedia::SegmentInfo>&	getMediaSegmentInfo() const
												{ return mMediaSegmentInfo; }
		const	OV<I<C> >&					getCodec() const
												{ return mCodec; }

	// Properties
	private:
		MTF						mMediaTrackFormat;
		OV<SMedia::SegmentInfo>	mMediaSegmentInfo;
		OV<I<C> >				mCodec;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaTrackInfos

class CMediaTrackInfos {
	// Types
	public:
		typedef	TMediaTrackInfo<SAudio::Format, CDecodeAudioCodec>	AudioTrackInfo;
		typedef	TMediaTrackInfo<SVideo::Format, CDecodeVideoCodec>	VideoTrackInfo;

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

			UniversalTimeInterval	getDuration() const
										{
											// Compose total duration
											UniversalTimeInterval	duration = 0.0;
											for (TIteratorD<AudioTrackInfo> iterator = mAudioTrackInfos.getIterator();
													iterator.hasValue(); iterator.advance()) {
												// Check for segment info
												if (iterator->getMediaSegmentInfo().hasValue())
													// Update duration
													duration =
															std::max<UniversalTimeInterval>(duration,
																	iterator->getMediaSegmentInfo()->getDuration());
											}
											for (TIteratorD<VideoTrackInfo> iterator = mVideoTrackInfos.getIterator();
													iterator.hasValue(); iterator.advance()) {
												// Check for segment info
												if (iterator->getMediaSegmentInfo().hasValue())
													// Update duration
													duration =
															std::max<UniversalTimeInterval>(duration,
																	iterator->getMediaSegmentInfo()->getDuration());
											}

											return duration;
										}

	// Properties
	private:
		TNArray<AudioTrackInfo>	mAudioTrackInfos;
		TNArray<VideoTrackInfo>	mVideoTrackInfos;
};
