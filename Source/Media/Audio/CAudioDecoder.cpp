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
							mAudioCodec(mAudioCodecInfo.instantiate())
			{}

				SAudioStorageFormat		mAudioStorageFormat;
		const	CAudioCodec::Info&		mAudioCodecInfo;
				I<CCodec::DecodeInfo>	mCodecDecodeInfo;
				I<CSeekableDataSource>	mSeekableDataSource;
				I<CAudioCodec>			mAudioCodec;
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
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mSeekableDataSource,
			mInternals->mCodecDecodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CAudioDecoder::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodec->decode(mediaPosition, audioFrames);
}
