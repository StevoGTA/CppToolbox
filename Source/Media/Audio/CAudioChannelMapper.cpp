//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioChannelMapper.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapperInternals

class CAudioChannelMapperInternals {
	public:
		typedef	void	(*PerformProc)(const CAudioFrames& sourceAudioFrames, CAudioFrames& destinationAudioFrames);

						CAudioChannelMapperInternals() : mPerformProc(nil) {}

		static	void	performMonoToStereoFloat32Interleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	Float32*			sourcePtr = (const Float32*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										Float32*			destinationPtr = (Float32*) writeInfo.getSegments()[0];

								// Perform
								for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}
		static	void	performMonoToStereoSInt16Interleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	SInt16*				sourcePtr = (const SInt16*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										SInt16*				destinationPtr = (SInt16*) writeInfo.getSegments()[0];

								// Perform
								for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}

		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>			mInputAudioFrames;
		PerformProc					mPerformProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::CAudioChannelMapper() : CBasicAudioProcessor()
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
OI<SError> CAudioChannelMapper::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup
	if ((mInternals->mInputAudioProcessingFormat->getChannelMap() == kAudioChannelMap_1_0) &&
			(mOutputAudioProcessingFormat->getChannelMap() == kAudioChannelMap_2_0_Option1)) {
		// Mono -> Stereo
		if ((audioProcessingFormat.getBits() == 32) && audioProcessingFormat.getIsFloat() &&
				audioProcessingFormat.getIsInterleaved())
			// Float32 interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereoFloat32Interleaved;
		else if ((audioProcessingFormat.getBits() == 16) && audioProcessingFormat.getIsSignedInteger() &&
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
SAudioSourceStatus CAudioChannelMapper::performInto(CAudioFrames& audioFrames)
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
	SAudioSourceStatus	audioSourceStatus = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	if (!audioSourceStatus.isSuccess())
		// Error
		return audioSourceStatus;

	// Perform
	mInternals->mPerformProc(*mInternals->mInputAudioFrames, audioFrames);

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
