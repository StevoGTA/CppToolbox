//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioChannelMapper.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapper::Internals

class CAudioChannelMapper::Internals {
	public:
		typedef	void	(*PerformProc)(const CAudioFrames& sourceAudioFrames, CAudioFrames& destinationAudioFrames,
								UInt32 sourceBytesPerFrame, UInt32 destinationBytesPerFrame);

						Internals() : mPerformProc(nil) {}

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

		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>				mInputAudioFrames;
		PerformProc						mPerformProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::CAudioChannelMapper() : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::~CAudioChannelMapper()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioChannelMapper::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup
	if ((mInternals->mInputAudioProcessingFormat->getChannelMap() == SAudio::ChannelMap::_1_0()) &&
			(mOutputAudioProcessingFormat->getChannelMap() == SAudio::ChannelMap::_2_0_Option1())) {
		// Mono -> Stereo
		if ((audioProcessingFormat.getBits() == 64) || (audioProcessingFormat.getBits() == 32) ||
				(audioProcessingFormat.getBits() == 24) || (audioProcessingFormat.getBits() == 16) ||
				(audioProcessingFormat.getBits() == 8))
			// Supported bits
			mInternals->mPerformProc = Internals::performMonoToStereo;
		else
			// Unsupported bits
			AssertFailUnimplemented();
	} else if (mInternals->mInputAudioProcessingFormat->getChannelMap().getChannelCount() >
			mOutputAudioProcessingFormat->getChannelMap().getChannelCount())
		// More -> Less
		mInternals->mPerformProc = Internals::performCopyCommon;
	else
		// Less -> More
		mInternals->mPerformProc = Internals::performSilenceExtra;

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioChannelMapper::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions +=
			indent + CString(OSSTR("Channel Mapper from ")) +
					CString::mDoubleQuotes +
							mInternals->mInputAudioProcessingFormat->getChannelMap().getDisplayString() +
							CString::mDoubleQuotes +
					CString(OSSTR(" to ")) +
					CString::mDoubleQuotes +
							mOutputAudioProcessingFormat->getChannelMap().getDisplayString() +
							CString::mDoubleQuotes;

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::SourceInfo> CAudioChannelMapper::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAllocatedFrameCount() != audioFrames.getAllocatedFrameCount())) {
		// Create Audio Data
		if (mInternals->mInputAudioProcessingFormat->getIsInterleaved())
			// Interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
									audioFrames.getAllocatedFrameCount()));
		else
			// Non-interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(mInternals->mInputAudioProcessingFormat->getChannelMap().getChannelCount(),
									mInternals->mInputAudioProcessingFormat->getBits() / 8,
									audioFrames.getAllocatedFrameCount()));
	} else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	TVResult<SMedia::SourceInfo>	mediaSourceInfo = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	if (mediaSourceInfo.hasError())
		// Error
		return mediaSourceInfo;

	// Perform
	mInternals->mPerformProc(*mInternals->mInputAudioFrames, audioFrames,
			mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
			mOutputAudioProcessingFormat->getBytesPerFrame());

	return mediaSourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMapper::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo(mOutputAudioProcessingFormat->getBits()),
					SAudio::ProcessingSetup::SampleRateInfo(mOutputAudioProcessingFormat->getSampleRate()),
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleTypeOption(
							mOutputAudioProcessingFormat->getIsFloat() ?
									SAudio::ProcessingSetup::kSampleTypeFloat :
									SAudio::ProcessingSetup::kSampleTypeSignedInteger),
					SAudio::ProcessingSetup::EndianOption(
							mOutputAudioProcessingFormat->getIsBigEndian() ?
									SAudio::ProcessingSetup::kEndianBig : SAudio::ProcessingSetup::kEndianLittle),
					SAudio::ProcessingSetup::InterleavedOption(
							mOutputAudioProcessingFormat->getIsInterleaved() ?
									SAudio::ProcessingSetup::kInterleaved : SAudio::ProcessingSetup::kNonInterleaved)));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMapper::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TSArray<SAudio::ProcessingSetup>(SAudio::ProcessingSetup::mUnspecified);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CAudioChannelMapper::canPerform(const SAudio::ChannelMap& fromAudioChannelMap, const SAudio::ChannelMap& toAudioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	return
			// Mono -> Stereo
			((fromAudioChannelMap == SAudio::ChannelMap::_1_0()) &&
					(toAudioChannelMap == SAudio::ChannelMap::_2_0_Option1())) ||

			// Stereo -> Mono
			((fromAudioChannelMap == SAudio::ChannelMap::_2_0_Option1()) &&
					(toAudioChannelMap == SAudio::ChannelMap::_1_0()));
}
