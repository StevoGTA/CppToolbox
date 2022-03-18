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

		static	void	performMonoToStereo4ByteInterleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	UInt32*				sourcePtr = (const UInt32*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										UInt32*				destinationPtr = (UInt32*) writeInfo.getSegments()[0];

								// Perform
								for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}
		static	void	performMonoToStereo3ByteInterleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	UInt8*				sourcePtr = (const UInt8*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										UInt8*				destinationPtr = (UInt8*) writeInfo.getSegments()[0];

								// Perform
								for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = *(sourcePtr + 1);
									(*destinationPtr++) = *(sourcePtr + 2);
									(*destinationPtr++) = (*sourcePtr++);
									(*destinationPtr++) = (*sourcePtr++);
									(*destinationPtr++) = (*sourcePtr++);
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}
		static	void	performMonoToStereo2ByteInterleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	UInt16*				sourcePtr = (const UInt16*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										UInt16*				destinationPtr = (UInt16*) writeInfo.getSegments()[0];

								// Perform
								for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
									// Copy sample
									(*destinationPtr++) = *sourcePtr;
									(*destinationPtr++) = (*sourcePtr++);
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}
		static	void	performMonoToStereo1ByteInterleaved(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	UInt8*				sourcePtr = (const UInt8*) readInfo.getSegments()[0];

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
										UInt8*				destinationPtr = (UInt8*) writeInfo.getSegments()[0];

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

// MARK: CAudioProcessor methods

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
		if ((audioProcessingFormat.getBits() == 32) && audioProcessingFormat.getIsInterleaved())
			// 4 byte interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereo4ByteInterleaved;
		else if ((audioProcessingFormat.getBits() == 24) && audioProcessingFormat.getIsInterleaved())
			// 3 byte interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereo3ByteInterleaved;
		else if ((audioProcessingFormat.getBits() == 16) && audioProcessingFormat.getIsInterleaved())
			// 2 byte interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereo2ByteInterleaved;
		else if ((audioProcessingFormat.getBits() == 8) && audioProcessingFormat.getIsInterleaved())
			// 1 byte interleaved
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereo1ByteInterleaved;
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
TNArray<CString> CAudioChannelMapper::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions = CBasicAudioProcessor::getSetupDescription(indent);

	// Add our setup description
	setupDescriptions +=
			indent + CString(OSSTR("Channel Mapper from ")) +
					eChannelMapGetDescription(mInternals->mInputAudioProcessingFormat->getChannelMap()) +
					CString(OSSTR(" to ")) + eChannelMapGetDescription(mOutputAudioProcessingFormat->getChannelMap());

	return setupDescriptions;
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
TArray<SAudioProcessingSetup> CAudioChannelMapper::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup::mUnspecified);
}

// MARK: Class methods

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
