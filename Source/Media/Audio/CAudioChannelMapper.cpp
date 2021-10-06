//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapperInternals

class CAudioChannelMapperInternals {
	public:
		typedef	void	(*PerformProc)(const CAudioFrames& sourceAudioFrames, CAudioFrames& destinationAudioFrames);

						CAudioChannelMapperInternals() : mPerformProc(nil) {}

		static	void	performMonoToStereoSInt16Interleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
								const	SInt16*	sourcePtr = (const SInt16*) (sourceAudioFrames.getSegmentsAsRead())[0];
										SInt16*	destinationPtr =
														(SInt16*) (destinationAudioFrames.getSegmentsAsWrite())[0];

								// Perform
								UInt32	frameCount = sourceAudioFrames.getCurrentFrameCount();
								for (UInt32 i = 0; i < frameCount; i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}
							}

		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>			mInputAudioFrames;
		OI<SAudioProcessingFormat>	mOutputAudioProcessingFormat;
		PerformProc					mPerformProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::CAudioChannelMapper() : CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioChannelMapperInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::~CAudioChannelMapper()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioChannelMapper::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup(*mInternals->mOutputAudioProcessingFormat));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioChannelMapper::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup
	if ((mInternals->mInputAudioProcessingFormat->getChannelMap() == kAudioChannelMap_1_0) &&
			(mInternals->mOutputAudioProcessingFormat->getChannelMap() == kAudioChannelMap_2_0_Option1)) {
		// Mono -> Stereo
		if ((audioProcessingFormat.getBits() == 16) && audioProcessingFormat.getIsSignedInteger() &&
				audioProcessingFormat.getIsInterleaved())
			// SInt16 interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereoSInt16Interleaved;
		else
			// Unimplemented
			AssertFailUnimplemented();
	} else {
		// Stereo -> Mono
		AssertFailUnimplemented();
	}

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioChannelMapper::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup::mUnspecified);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioChannelMapper::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mOutputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CAudioChannelMapper::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAvailableFrameCount() != audioFrames.getAvailableFrameCount())) {
		// Create or reset Audio Data
		if (mInternals->mInputAudioProcessingFormat->getIsInterleaved())
			// Interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(1, mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
									audioFrames.getAvailableFrameCount()));
		else
			// Non-interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(mInternals->mInputAudioProcessingFormat->getChannels(),
									mInternals->mInputAudioProcessingFormat->getBits() / 8,
									audioFrames.getAvailableFrameCount()));
	} else
		// Use existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	SAudioSourceStatus	audioSourceStatus = CAudioProcessor::perform(mediaPosition, *mInternals->mInputAudioFrames);
	if (!audioSourceStatus.isSuccess())
		// Error
		return audioSourceStatus;

	// Perform
	mInternals->mPerformProc(*mInternals->mInputAudioFrames, audioFrames);
	audioFrames.completeWrite(mInternals->mInputAudioFrames->getCurrentFrameCount());

	return audioSourceStatus;
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioChannelMapper::canPerform(EAudioChannelMap fromAudioChannelMap, EAudioChannelMap toAudioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	return
			// Mono -> Stereo
			((fromAudioChannelMap == EAudioChannelMap::kAudioChannelMap_1_0) &&
					(toAudioChannelMap == EAudioChannelMap::kAudioChannelMap_2_0_Option1)) ||

			// Stereo -> Mono
			((fromAudioChannelMap == EAudioChannelMap::kAudioChannelMap_2_0_Option1) &&
					(toAudioChannelMap == EAudioChannelMap::kAudioChannelMap_1_0));

}
