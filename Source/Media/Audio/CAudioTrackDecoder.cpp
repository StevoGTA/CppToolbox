//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrackDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioTrackDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrackDecoderInternals

class CAudioTrackDecoderInternals : public TReferenceCountable<CAudioTrackDecoderInternals> {
	public:
		CAudioTrackDecoderInternals(const CAudioTrack& audioTrack, const I<CDataSource>& dataSource) :
			TReferenceCountable(),
					mAudioCodecInfo(
							CCodecRegistry::mShared.getAudioCodecInfo(audioTrack.getAudioStorageFormat().getCodecID())),
					mAudioTrack(audioTrack), mDataSource(dataSource), mAudioCodec(mAudioCodecInfo.instantiate())
			{}

		const	CAudioCodec::Info&	mAudioCodecInfo;
				CAudioTrack			mAudioTrack;
				I<CDataSource>		mDataSource;
				I<CAudioCodec>		mAudioCodec;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioTrackDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackDecoder::CAudioTrackDecoder(const CAudioTrack& audioTrack, const I<CDataSource>& dataSource) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioTrackDecoderInternals(audioTrack, dataSource);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackDecoder::CAudioTrackDecoder(const CAudioTrackDecoder& other) : CAudioSource(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackDecoder::~CAudioTrackDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioTrackDecoder::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodecInfo.getAudioProcessingSetups(mInternals->mAudioTrack.getAudioStorageFormat());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioTrackDecoder::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mDataSource,
			mInternals->mAudioTrack.getDecodeInfo());
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioTrackDecoder::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodec->decode(mediaPosition, audioFrames);
}
