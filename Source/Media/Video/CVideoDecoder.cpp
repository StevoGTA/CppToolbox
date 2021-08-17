//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoderInternals

class CVideoDecoderInternals : public TReferenceCountable<CVideoDecoderInternals> {
	public:
		CVideoDecoderInternals(const SVideoStorageFormat& videoStorageFormat,
				const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CSeekableDataSource>& seekableDataSource) :
			TReferenceCountable(),
					mVideoStorageFormat(videoStorageFormat),
					mCodecDecodeInfo(codecDecodeInfo), mSeekableDataSource(seekableDataSource),
					mVideoCodec(
							CCodecRegistry::mShared.getVideoCodecInfo(videoStorageFormat.getCodecID()).instantiate()),
					mMediaReader(codecDecodeInfo->createMediaReader(seekableDataSource))
			{}

		SVideoStorageFormat		mVideoStorageFormat;
		I<CCodec::DecodeInfo>	mCodecDecodeInfo;
		I<CSeekableDataSource>	mSeekableDataSource;
		I<CVideoCodec>			mVideoCodec;
		I<CMediaReader>			mMediaReader;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
		const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CSeekableDataSource>& seekableDataSource,
		CVideoFrame::Compatibility compatibility) : CVideoSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CVideoDecoderInternals(videoStorageFormat, codecDecodeInfo, seekableDataSource);

	// Setup Video Codec
	mInternals->mVideoCodec->setupForDecode(mInternals->mMediaReader, mInternals->mCodecDecodeInfo, compatibility);
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
CVideoProcessor::PerformResult CVideoDecoder::perform(const SMediaPosition& mediaPosition)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Float32		percentConsumed = mInternals->mMediaReader->getPercentConsumed();
	OI<SError>	error;

	// Update read position if needed
	if (mediaPosition.getMode() != SMediaPosition::kFromCurrent) {
		// Reset audio codec
		mInternals->mVideoCodec->decodeReset();

//		// Set media position
//		error = mInternals->mMediaReader->set(mediaPosition, *mInternals->mAudioProcessingFormat);
//		ReturnValueIfError(error, SVideoSourceStatus(*error));
	}

	// Decode
	TIResult<CVideoFrame>	videoFrame = mInternals->mVideoCodec->decode();
	ReturnValueIfResultError(videoFrame, PerformResult(videoFrame.getError()));

	return PerformResult(percentConsumed, videoFrame.getValue());
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CVideoDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset audio codec
	mInternals->mVideoCodec->decodeReset();

	return OI<SError>();
}
