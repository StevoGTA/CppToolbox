//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoderInternals

class CVideoDecoderInternals : public TReferenceCountable<CVideoDecoderInternals> {
	public:
		CVideoDecoderInternals(const SVideoStorageFormat& videoStorageFormat, const I<CVideoCodec>& videoCodec,
				const I<CCodec::DecodeInfo>& codecDecodeInfo) :
			TReferenceCountable(),
					mVideoStorageFormat(videoStorageFormat), mVideoCodec(videoCodec), mCodecDecodeInfo(codecDecodeInfo),
					mStartTimeInterval(0.0), mCurrentTimeInterval(0.0)
			{}

		SVideoStorageFormat			mVideoStorageFormat;
		I<CVideoCodec>				mVideoCodec;
		I<CCodec::DecodeInfo>		mCodecDecodeInfo;

		OI<SVideoProcessingFormat>	mVideoProcessingFormat;
		UniversalTimeInterval		mStartTimeInterval;
		OV<UniversalTimeInterval>	mDurationTimeInterval;
		UniversalTimeInterval		mCurrentTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const SVideoStorageFormat& videoStorageFormat, const I<CVideoCodec>& videoCodec,
		const I<CCodec::DecodeInfo>& codecDecodeInfo, const SVideoProcessingFormat& videoProcessingFormat) :
		CVideoSource()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CVideoDecoderInternals(videoStorageFormat, videoCodec, codecDecodeInfo);

	// Store
	mInternals->mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);

	// Setup Video Codec
	mInternals->mVideoCodec->setupForDecode(videoProcessingFormat, mInternals->mCodecDecodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const CVideoDecoder& other) : CVideoSource(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::~CVideoDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::setSourceWindow(UniversalTimeInterval startTimeInterval,
		const OV<UniversalTimeInterval>& durationTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	performSeek = startTimeInterval != mInternals->mStartTimeInterval;

	// Store
	mInternals->mStartTimeInterval = startTimeInterval;
	mInternals->mDurationTimeInterval = OV<UniversalTimeInterval>(durationTimeInterval);

	// Check if need seek
	if (performSeek)
		// Seek
		seek(mInternals->mStartTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Bound the given time
	timeInterval = std::max<UniversalTimeInterval>(timeInterval, mInternals->mStartTimeInterval);
	if (mInternals->mDurationTimeInterval.hasValue())
		timeInterval =
				std::min<UniversalTimeInterval>(timeInterval,
						mInternals->mStartTimeInterval + *mInternals->mDurationTimeInterval);

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;

	// Inform codec
	mInternals->mVideoCodec->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::PerformResult CVideoDecoder::perform()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UniversalTimeInterval	currentTimeInterval = mInternals->mCurrentTimeInterval;

	// Decode
	TIResult<CVideoFrame>	videoFrame = mInternals->mVideoCodec->decode();
	ReturnValueIfResultError(videoFrame, PerformResult(videoFrame.getError()));

	// Update
	currentTimeInterval += 1.0 / mInternals->mVideoProcessingFormat->getFramerate();

	return PerformResult(currentTimeInterval, videoFrame.getValue());
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mInternals->mVideoCodec->seek(mInternals->mStartTimeInterval);
	mInternals->mCurrentTimeInterval = mInternals->mStartTimeInterval;
}
