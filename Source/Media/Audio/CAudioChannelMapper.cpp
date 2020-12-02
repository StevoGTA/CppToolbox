//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapperInternals

class CAudioChannelMapperInternals {
	public:
		typedef	void	(*PerformProc)(const CAudioData& sourceData, CAudioData& destinationData);

						CAudioChannelMapperInternals() : mPerformProc(nil) {}

		static	void	performMonoToStereoSInt16Interleaved(const CAudioData& sourceData, CAudioData& destinationData)
							{
								// Setup
								SInt16*	sourcePtr = (SInt16*) (*sourceData.getBuffers())[0];
								SInt16*	destinationPtr = (SInt16*) (*destinationData.getBuffers())[0];

								// Perform
								UInt32	frameCount = sourceData.getCurrentFrameCount();
								for (UInt32 i = 0; i < frameCount; i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}
							}

		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioData>				mInputAudioData;
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
SAudioReadStatus CAudioChannelMapper::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioData.hasInstance() ||
			(mInternals->mInputAudioData->getAvailableFrameCount() != audioData.getAvailableFrameCount())) {
		// Create or reset Audio Data
		if (mInternals->mInputAudioProcessingFormat->getIsInterleaved())
			// Interleaved
			mInternals->mInputAudioData =
					OI<CAudioData>(
							new CAudioData(1, mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
									audioData.getAvailableFrameCount()));
		else
			// Non-interleaved
			mInternals->mInputAudioData =
					OI<CAudioData>(
							new CAudioData(mInternals->mInputAudioProcessingFormat->getChannels(),
									mInternals->mInputAudioProcessingFormat->getBits() / 8,
									audioData.getAvailableFrameCount()));
	} else
		// Use existing Audio Data
		mInternals->mInputAudioData->reset();

	// Read
	SAudioReadStatus	audioReadStatus = CAudioProcessor::perform(mediaPosition, *mInternals->mInputAudioData);
	if (!audioReadStatus.isSuccess())
		// Error
		return audioReadStatus;

	// Perform
	mInternals->mPerformProc(*mInternals->mInputAudioData, audioData);
	audioData.completeWrite(mInternals->mInputAudioData->getCurrentFrameCount());

	return audioReadStatus;
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
