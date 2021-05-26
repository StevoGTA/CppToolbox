//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoDecoder.h"

#include "CCodecRegistry.h"
#include "TLockingArray.h"

#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoderInternals

class CVideoDecoderInternals {
	public:
						CVideoDecoderInternals(CVideoDecoder& videoDecoder, const CVideoTrack& videoTrack,
								const I<CDataSource>& dataSource, UInt32 trackIndex,
								const CVideoCodec::Info& videoCodecInfo, const CVideoDecoder::DecodeInfo& decodeInfo,
								CVideoCodec::DecodeFrameInfo::Compatibility compatibility,
								const CVideoDecoder::RenderInfo& renderInfo) :
							mVideoDecoder(videoDecoder), mDecodeInfo(decodeInfo), mRenderInfo(renderInfo),
									mVideoCodec(videoCodecInfo.instantiate()), mTrackIndex(trackIndex),
									mNotifiedFirstFrameReady(false), mDecodeInProgress(false), mResetPending(false)
							{
								// Setup for decode
								mVideoCodec->setupForDecode(dataSource, videoTrack.getDecodeInfo(),
										CVideoCodec::DecodeFrameInfo(compatibility, frameReady, error, this));
							}
						~CVideoDecoderInternals()
							{
								// Reset video codec
								mVideoCodec->reset();
							}

				void	triggerDecode()
							{
								// Trigger decode
								if (mVideoCodec->triggerDecode())
									// Decode in progress
									mDecodeInProgress = true;
							}
				void	triggerReset()
							{
								// Dump all video frames
								mVideoFrames.removeAll();

								// Reset
								mVideoCodec->reset();

								// Reset handled
								mNotifiedFirstFrameReady = false;
								mResetPending = false;

								// Queue first frame
								triggerDecode();
							}

		static	void	frameReady(const CVideoFrame& videoFrame, void* userData)
							{
								// Setup
								CVideoDecoderInternals&	internals = *((CVideoDecoderInternals*) userData);

								// Add and order
								internals.mVideoFrames.add(videoFrame);
								internals.mVideoFrames.sort(CVideoFrame::comparePresentationTimeInterval);

								// Decode finished
								internals.mDecodeInProgress = false;

								// Check for reset pending
								if (internals.mResetPending)
									// Must reset
									internals.triggerReset();
								else
									// Call proc
									internals.mDecodeInfo.frameReady(internals.mVideoDecoder);
							}
		static	void	error(const SError& error, void* userData)
							{
								// Setup
								CVideoDecoderInternals&	internals = *((CVideoDecoderInternals*) userData);

								// Decode finished
								internals.mDecodeInProgress = false;

								// Check for reset pending
								if (internals.mResetPending)
									// Must reset
									internals.triggerReset();
								else
									// Call proc
									internals.mDecodeInfo.error(internals.mVideoDecoder, error);
							}


		CVideoDecoder&				mVideoDecoder;
		CVideoDecoder::DecodeInfo	mDecodeInfo;
		CVideoDecoder::RenderInfo	mRenderInfo;
		I<CVideoCodec>				mVideoCodec;
		UInt32						mTrackIndex;

		TNLockingArray<CVideoFrame>	mVideoFrames;
		bool						mNotifiedFirstFrameReady;
		bool						mDecodeInProgress;
		bool						mResetPending;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const CVideoTrack& videoTrack, const I<CDataSource>& dataSource, const CString& identifier,
		UInt32 trackIndex, const DecodeInfo& decodeInfo, CVideoCodec::DecodeFrameInfo::Compatibility compatibility,
		const RenderInfo& renderInfo) : CVideoProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CVideoDecoderInternals(*this, videoTrack, dataSource, trackIndex,
					CCodecRegistry::mShared.getVideoCodecInfo(videoTrack.getVideoStorageFormat().getCodecID()),
					decodeInfo, compatibility, renderInfo);

	// Queue first frame
	mInternals->triggerDecode();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::~CVideoDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CVideoDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset pending
	mInternals->mResetPending = true;

	// Check situation
	if (!mInternals->mDecodeInProgress)
		// Can do reset now
		mInternals->triggerReset();

	return OI<SError>();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::handleFrameReady()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if notified first frame ready
	if (!mInternals->mNotifiedFirstFrameReady) {
		// Call proc
		mInternals->mRenderInfo.currentVideoFrameUpdated(mInternals->mTrackIndex, mInternals->mVideoFrames[0]);
		mInternals->mNotifiedFirstFrameReady = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::notePositionUpdated(UniversalTimeInterval position)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if time to go to next frame
	if ((mInternals->mVideoFrames.getCount() > 1) &&
			(position >= mInternals->mVideoFrames[1].getPresentationTimeInterval())) {
		// Next frame!
		mInternals->mVideoFrames.removeAtIndex(0);
		mInternals->mRenderInfo.currentVideoFrameUpdated(mInternals->mTrackIndex, mInternals->mVideoFrames[0]);
	}

	// Check if need to queue more frames
	if (!mInternals->mDecodeInProgress && (mInternals->mVideoFrames.getCount() < 10))
		// Trigger decode
		mInternals->triggerDecode();
}
