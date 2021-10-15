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
											const I<CAudioCodec>& audioCodec,
											const I<CCodec::DecodeInfo>& codecDecodeInfo) :
										TReferenceCountable(),
												mAudioStorageFormat(audioStorageFormat), mAudioCodec(audioCodec),
												mCodecDecodeInfo(codecDecodeInfo),
												mStartTimeInterval(0.0), mCurrentTimeInterval(0.0)
										{}

		OV<UniversalTimeInterval>	getEndTimeInterval()
										{ return mDurationTimeInterval.hasValue() ?
												OV<UniversalTimeInterval>(mStartTimeInterval + *mDurationTimeInterval) :
												OV<UniversalTimeInterval>(); }

		SAudioStorageFormat			mAudioStorageFormat;
		I<CAudioCodec>				mAudioCodec;
		I<CCodec::DecodeInfo>		mCodecDecodeInfo;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
		UniversalTimeInterval		mStartTimeInterval;
		OV<UniversalTimeInterval>	mDurationTimeInterval;
		UniversalTimeInterval		mCurrentTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioDecoder::CAudioDecoder(const SAudioStorageFormat& audioStorageFormat, const I<CAudioCodec>& audioCodec,
		const I<CCodec::DecodeInfo>& codecDecodeInfo) : CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioDecoderInternals(audioStorageFormat, audioCodec, codecDecodeInfo);
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
	return CCodecRegistry::mShared.getAudioCodecInfo(mInternals->mAudioStorageFormat.getCodecID())
			.getAudioProcessingSetups(mInternals->mAudioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	
	// Setup Audio Codec
	mInternals->mAudioCodec->setupForDecode(audioProcessingFormat, mInternals->mCodecDecodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioProcessor::Requirements CAudioDecoder::queryRequirements() const
//----------------------------------------------------------------------------------------------------------------------
{
	return Requirements(mInternals->mAudioCodec->getRequirements());
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
		timeInterval = std::min<UniversalTimeInterval>(timeInterval, *mInternals->getEndTimeInterval());

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;

	// Inform codec
	mInternals->mAudioCodec->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CAudioDecoder::performInto(CAudioFrames& audioFrames)
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
			return SAudioSourceStatus(SError::mEndOfData);

		// Calculate
		maxFrames = (UInt32) (durationRemaining * mInternals->mAudioProcessingFormat->getSampleRate());
	} else
		// No duration
		maxFrames = ~0;

	// Setup
	SAudioSourceStatus	audioSourceStatus(mInternals->mCurrentTimeInterval);

	// Decode
	OI<SError>	error = mInternals->mAudioCodec->decode(audioFrames);
	ReturnValueIfError(error, SAudioSourceStatus(*error));

	// Limit to max frames
	audioFrames.limit(maxFrames);

	// Update
	mInternals->mCurrentTimeInterval +=
			(UniversalTimeInterval) audioFrames.getCurrentFrameCount() /
					mInternals->mAudioProcessingFormat->getSampleRate();

	return audioSourceStatus;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mInternals->mAudioCodec->seek(mInternals->mStartTimeInterval);
	mInternals->mCurrentTimeInterval = mInternals->mStartTimeInterval;
}
