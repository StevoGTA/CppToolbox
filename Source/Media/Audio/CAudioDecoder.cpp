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
				const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CDataSource>& dataSource) :
			TReferenceCountable(),
					mAudioStorageFormat(audioStorageFormat),
					mAudioCodecInfo(CCodecRegistry::mShared.getAudioCodecInfo(mAudioStorageFormat.getCodecID())),
					mCodecDecodeInfo(codecDecodeInfo), mDataSource(dataSource),
							mAudioCodec(mAudioCodecInfo.instantiate())
			{}

				SAudioStorageFormat		mAudioStorageFormat;
		const	CAudioCodec::Info&		mAudioCodecInfo;
				I<CCodec::DecodeInfo>	mCodecDecodeInfo;
				I<CDataSource>			mDataSource;
				I<CAudioCodec>			mAudioCodec;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::CAudioDecoder(const SAudioStorageFormat& audioStorageFormat,
		const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CDataSource>& dataSource) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioDecoderInternals(audioStorageFormat, codecDecodeInfo, dataSource);
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
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mDataSource,
			mInternals->mCodecDecodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioDecoder::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodec->decode(mediaPosition, audioFrames);
}
