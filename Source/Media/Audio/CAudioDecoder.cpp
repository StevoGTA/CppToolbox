//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoderInternals

class CAudioDecoderInternals : public TReferenceCountable<CAudioDecoderInternals> {
	public:
		CAudioDecoderInternals(const SAudioStorageFormat& audioStorageFormat,
				const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CSeekableDataSource>& seekableDataSource) :
			TReferenceCountable(),
					mAudioStorageFormat(audioStorageFormat),
					mAudioCodecInfo(CCodecRegistry::mShared.getAudioCodecInfo(mAudioStorageFormat.getCodecID())),
					mCodecDecodeInfo(codecDecodeInfo), mSeekableDataSource(seekableDataSource),
					mAudioCodec(mAudioCodecInfo.instantiate()),
					mMediaReader(codecDecodeInfo->createMediaReader(seekableDataSource))
			{}

				SAudioStorageFormat			mAudioStorageFormat;
		const	CAudioCodec::Info&			mAudioCodecInfo;
				I<CCodec::DecodeInfo>		mCodecDecodeInfo;
				I<CSeekableDataSource>		mSeekableDataSource;
				I<CAudioCodec>				mAudioCodec;
				I<CMediaReader>				mMediaReader;
				OI<SAudioProcessingFormat>	mAudioProcessingFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::CAudioDecoder(const SAudioStorageFormat& audioStorageFormat,
		const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CSeekableDataSource>& seekableDataSource) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioDecoderInternals(audioStorageFormat, codecDecodeInfo, seekableDataSource);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::CAudioDecoder(const CAudioDecoder& other) : CAudioSource(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::~CAudioDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioDecoder::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodecInfo.getAudioProcessingSetups(mInternals->mAudioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup Audio Codec
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mMediaReader,
			mInternals->mCodecDecodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CAudioDecoder::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Float32		percentConsumed = mInternals->mMediaReader->getPercentConsumed();
	OI<SError>	error;

	// Update read position if needed
	if (mediaPosition.getMode() != SMediaPosition::kFromCurrent) {
		// Reset audio codec
		mInternals->mAudioCodec->decodeReset();

		// Set media position
		error = mInternals->mMediaReader->set(mediaPosition, *mInternals->mAudioProcessingFormat);
		ReturnValueIfError(error, SAudioSourceStatus(*error));
	}

	// Decode
	error = mInternals->mAudioCodec->decode(audioFrames);
	ReturnValueIfError(error, SAudioSourceStatus(*error));

	return SAudioSourceStatus(percentConsumed);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset audio codec
	mInternals->mAudioCodec->decodeReset();

	return OI<SError>();
}
