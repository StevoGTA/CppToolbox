//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioChannelMapper.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapperInternals

class CAudioChannelMapperInternals {
	public:
		typedef	void	(*PerformProc)(const CAudioFrames& sourceAudioFrames, CAudioFrames& destinationAudioFrames,
								UInt32 sourceBytesPerFrame, UInt32 destinationBytesPerFrame);

						CAudioChannelMapperInternals() : mPerformProc(nil) {}

		static	void	performCopyCommon(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames, UInt32 sourceBytesPerFrame,
								UInt32 destinationBytesPerFrame)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	TNumberArray<void*>	readInfoSegments = readInfo.getSegments();

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
								const	TNumberArray<void*>	writeInfoSegments = writeInfo.getSegments();

								// Check interleaved
								if (readInfoSegments.getCount() == 1) {
									// Interleaved
									const	UInt8*	sourcePtr = (const UInt8*) readInfoSegments[0];
											UInt8*	destinationPtr = (UInt8*) writeInfo.getSegments()[0];
									for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
										// Copy frame
										::memcpy(destinationPtr, sourcePtr, destinationBytesPerFrame);
										sourcePtr += sourceBytesPerFrame;
										destinationPtr += destinationBytesPerFrame;
									}
								} else {
									// Non-interleaved
									AssertFailUnimplemented();
								}

								// Complete
								destinationAudioFrames.completeWrite(readInfo.getFrameCount());
							}
		static	void	performSilenceExtra(const CAudioFrames& sourceAudioFrames,
								CAudioFrames& destinationAudioFrames, UInt32 sourceBytesPerFrame,
								UInt32 destinationBytesPerFrame)
							{
							}

		static	void	performMonoToStereo(const CAudioFrames& sourceAudioFrames, CAudioFrames& destinationAudioFrames,
								UInt32 sourceBytesPerSample, UInt32 destinationBytesPerSample)
							{
								// Setup
										CAudioFrames::Info	readInfo = sourceAudioFrames.getReadInfo();
								const	TNumberArray<void*>	readInfoSegments = readInfo.getSegments();

										CAudioFrames::Info	writeInfo = destinationAudioFrames.getWriteInfo();
								const	TNumberArray<void*>	writeInfoSegments = writeInfo.getSegments();

								// Check interleaved
								if (readInfoSegments.getCount() == 1) {
									// Interleaved
									switch (sourceBytesPerSample) {
										case 8: {
											// 8 bytes per sample
											const	UInt64*	sourcePtr = (const UInt64*) readInfoSegments[0];
													UInt64*	destinationPtr = (UInt64*) writeInfoSegments[0];
											for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
												// Copy sample
												(*destinationPtr++) = *sourcePtr;
												(*destinationPtr++) = (*sourcePtr++);
											}
											} break;

										case 4: {
											// 4 bytes per sample
											const	UInt32*	sourcePtr = (const UInt32*) readInfoSegments[0];
													UInt32*	destinationPtr = (UInt32*) writeInfoSegments[0];
											for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
												// Copy sample
												(*destinationPtr++) = *sourcePtr;
												(*destinationPtr++) = (*sourcePtr++);
											}
											} break;

										case 3: {
											// 3 bytes per sample
											const	UInt8*	sourcePtr = (const UInt8*) readInfoSegments[0];
													UInt8*	destinationPtr = (UInt8*) writeInfoSegments[0];
											for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
												// Copy sample
												(*destinationPtr++) = *sourcePtr;
												(*destinationPtr++) = *(sourcePtr + 1);
												(*destinationPtr++) = *(sourcePtr + 2);
												(*destinationPtr++) = (*sourcePtr++);
												(*destinationPtr++) = (*sourcePtr++);
												(*destinationPtr++) = (*sourcePtr++);
											}
											} break;

										case 2: {
											// 2 bytes per sample
											const	UInt16*	sourcePtr = (const UInt16*) readInfoSegments[0];
													UInt16*	destinationPtr = (UInt16*) writeInfoSegments[0];

											// Perform
											for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
												// Copy sample
												(*destinationPtr++) = *sourcePtr;
												(*destinationPtr++) = (*sourcePtr++);
											}
											} break;

										case 1: {
											// 1 byte per sample
											const	UInt8*	sourcePtr = (const UInt8*) readInfoSegments[0];
													UInt8*	destinationPtr = (UInt8*) writeInfoSegments[0];
											for (UInt32 i = 0; i < readInfo.getFrameCount(); i++) {
												// Copy sample
												(*destinationPtr++) = *sourcePtr;
												(*destinationPtr++) = (*sourcePtr++);
											}
											} break;
									}
								} else {
									// Non-interleaved
									AssertFailUnimplemented();
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
	if ((mInternals->mInputAudioProcessingFormat->getAudioChannelMap() == kAudioChannelMap_1_0) &&
			(mOutputAudioProcessingFormat->getAudioChannelMap() == kAudioChannelMap_2_0_Option1)) {
		// Mono -> Stereo
		if ((audioProcessingFormat.getBits() == 64) || (audioProcessingFormat.getBits() == 32) ||
				(audioProcessingFormat.getBits() == 24) || (audioProcessingFormat.getBits() == 16) ||
				(audioProcessingFormat.getBits() == 8))
			// Supported bits
			mInternals->mPerformProc = CAudioChannelMapperInternals::performMonoToStereo;
		else
			// Unsupported bits
			AssertFailUnimplemented();
	} else if (mInternals->mInputAudioProcessingFormat->getChannels() > mOutputAudioProcessingFormat->getChannels())
		// More -> Less
		mInternals->mPerformProc = CAudioChannelMapperInternals::performCopyCommon;
	else
		// Less -> More
		mInternals->mPerformProc = CAudioChannelMapperInternals::performSilenceExtra;

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
					CString::mDoubleQuotes +
							eChannelMapGetDescription(mInternals->mInputAudioProcessingFormat->getAudioChannelMap()) +
							CString::mDoubleQuotes +
					CString(OSSTR(" to ")) +
					CString::mDoubleQuotes +
							eChannelMapGetDescription(mOutputAudioProcessingFormat->getAudioChannelMap()) +
							CString::mDoubleQuotes;

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
	mInternals->mPerformProc(*mInternals->mInputAudioFrames, audioFrames,
			mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
			mOutputAudioProcessingFormat->getBytesPerFrame());

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
