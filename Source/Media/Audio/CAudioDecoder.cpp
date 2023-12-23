//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoder::Internals

class CAudioDecoder::Internals {
	public:
		Internals(const SAudio::Format& audioFormat, const I<CDecodeAudioCodec>& decodeAudioCodec,
				const CString& identifier) :
			mAudioFormat(audioFormat), mDecodeAudioCodec(decodeAudioCodec), mIdentifier(identifier)
			{}

		SAudio::Format					mAudioFormat;
		I<CDecodeAudioCodec>			mDecodeAudioCodec;
		CString							mIdentifier;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::CAudioDecoder(const SAudio::Format& audioFormat, const I<CDecodeAudioCodec>& audioCodec,
		const CString& identifier) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(audioFormat, audioCodec, identifier);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::~CAudioDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioDecoder::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CAudioSource::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Audio Decoder (")) + mInternals->mIdentifier + CString(OSSTR(")"));
	setupDescriptions +=
			indent + CString(OSSTR("    ")) +
					CCodecRegistry::mShared.getAudioCodecInfo(mInternals->mAudioFormat.getCodecID()).getDecodeName() +
					CString(OSSTR(", ")) + mInternals->mAudioFormat.getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Requirements CAudioDecoder::queryRequirements() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDecodeAudioCodec->getRequirements();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CAudioSource::seek(timeInterval);

	// Update codec
	mInternals->mDecodeAudioCodec->seek(getCurrentTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CAudioDecoder::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get max frames
	TVResult<UInt32>	maxFrames = calculateMaxFrames(mInternals->mAudioProcessingFormat->getSampleRate());
	ReturnValueIfResultError(maxFrames, TVResult<SourceInfo>(maxFrames.getError()));

	// Setup
	TVResult<SourceInfo>	sourceInfo(getCurrentTimeInterval());

	// Decode
	OV<SError>	error = mInternals->mDecodeAudioCodec->decodeInto(audioFrames);
	ReturnValueIfError(error, TVResult<SourceInfo>(*error));

	// Limit to max frames
	audioFrames.limit(*maxFrames);

	// Update
	setCurrentTimeInterval(
			getCurrentTimeInterval() +
					(UniversalTimeInterval) audioFrames.getCurrentFrameCount() /
							mInternals->mAudioProcessingFormat->getSampleRate());

	return sourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CAudioSource::reset();

	// Update codec
	mInternals->mDecodeAudioCodec->seek(getCurrentTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioDecoder::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDecodeAudioCodec->getAudioProcessingSetups(mInternals->mAudioFormat);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioDecoder::setOutputFormat(const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup Audio Codec
	return mInternals->mDecodeAudioCodec->setup(audioProcessingFormat);
}
