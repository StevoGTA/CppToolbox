//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioDecoder.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoder::Internals

class CAudioDecoder::Internals {
	public:
									Internals(const SAudio::Format& audioFormat,
											const I<CDecodeAudioCodec>& decodeAudioCodec, const CString& identifier) :
										mAudioFormat(audioFormat), mDecodeAudioCodec(decodeAudioCodec),
												mIdentifier(identifier),
												mStartTimeInterval(0.0), mCurrentTimeInterval(0.0)
										{}

		OV<UniversalTimeInterval>	getEndTimeInterval()
										{ return mDurationTimeInterval.hasValue() ?
												OV<UniversalTimeInterval>(mStartTimeInterval + *mDurationTimeInterval) :
												OV<UniversalTimeInterval>(); }

		SAudio::Format					mAudioFormat;
		I<CDecodeAudioCodec>			mDecodeAudioCodec;
		CString							mIdentifier;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;
		UniversalTimeInterval			mStartTimeInterval;
		OV<UniversalTimeInterval>		mDurationTimeInterval;
		UniversalTimeInterval			mCurrentTimeInterval;
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
void CAudioDecoder::setSourceWindow(UniversalTimeInterval startTimeInterval,
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
void CAudioDecoder::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Bound the given time
	timeInterval = std::max<UniversalTimeInterval>(timeInterval, mInternals->mStartTimeInterval);
	if (mInternals->mDurationTimeInterval.hasValue())
		// Limit to duration
		timeInterval = std::min<UniversalTimeInterval>(timeInterval, *mInternals->getEndTimeInterval());

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;

	// Inform codec
	mInternals->mDecodeAudioCodec->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::SourceInfo> CAudioDecoder::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	maxFrames;
	if (mInternals->mDurationTimeInterval.hasValue()) {
		// Have duration
		UniversalTimeInterval	durationRemaining =
										*mInternals->getEndTimeInterval() - mInternals->mCurrentTimeInterval;
		if (durationRemaining <= 0.0)
			// Already done
			return TVResult<SMedia::SourceInfo>(SError::mEndOfData);

		// Calculate
		maxFrames = (UInt32) (durationRemaining * mInternals->mAudioProcessingFormat->getSampleRate());
	} else
		// No duration
		maxFrames = (UInt32) ~0;

	// Setup
	TVResult<SMedia::SourceInfo>	mediaSourceInfo(mInternals->mCurrentTimeInterval);

	// Decode
	OV<SError>	error = mInternals->mDecodeAudioCodec->decodeInto(audioFrames);
	ReturnValueIfError(error, TVResult<SMedia::SourceInfo>(*error));

	// Limit to max frames
	audioFrames.limit(maxFrames);

	// Update
	mInternals->mCurrentTimeInterval +=
			(UniversalTimeInterval) audioFrames.getCurrentFrameCount() /
					mInternals->mAudioProcessingFormat->getSampleRate();

	return mediaSourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mInternals->mDecodeAudioCodec->seek(mInternals->mStartTimeInterval);
	mInternals->mCurrentTimeInterval = mInternals->mStartTimeInterval;
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
