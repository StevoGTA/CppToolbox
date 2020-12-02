//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrackReader.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioTrackReader.h"

#include "CAudioCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrackReaderInternals

class CAudioTrackReaderInternals : public TReferenceCountable<CAudioTrackReaderInternals> {
	public:
		CAudioTrackReaderInternals(const CAudioTrack& audioTrack, CByteParceller& byteParceller) :
			TReferenceCountable(),
					mAudioCodecInfo(
							CAudioCodecRegistry::mShared.getInfo(audioTrack.getAudioStorageFormat().getCodecID())),
					mAudioTrack(audioTrack), mByteParceller(byteParceller), mAudioCodec(mAudioCodecInfo.instantiate())
			{}

		const	CAudioCodec::SInfo&	mAudioCodecInfo;
				CAudioTrack			mAudioTrack;
				CByteParceller		mByteParceller;
				I<CAudioCodec>		mAudioCodec;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioTrackReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackReader::CAudioTrackReader(const CAudioTrack& audioTrack, CByteParceller& byteParceller) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioTrackReaderInternals(audioTrack, byteParceller);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackReader::CAudioTrackReader(const CAudioTrackReader& other) : CAudioSource(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrackReader::~CAudioTrackReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioTrackReader::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodecInfo.getAudioProcessingSetups(mInternals->mAudioTrack.getAudioStorageFormat());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioTrackReader::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mByteParceller,
			mInternals->mAudioTrack.getDecodeInfo());
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioTrackReader::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioCodec->decode(mediaPosition, audioData);
}
