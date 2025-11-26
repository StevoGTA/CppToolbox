//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioAudioConverter.h"

#include "SError-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioAudioConverter::Internals

class CCoreAudioAudioConverter::Internals {
	public:
							Internals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mOutputAudioBufferList(nil), mAudioConverterRef(nil),
										mSourceHasMoreToRead(true), mSourceTimeInterval(0.0)
								{}
							~Internals()
								{
									// Cleanup
									::free(mOutputAudioBufferList);
									if (mAudioConverterRef != nil)
										::AudioConverterDispose(mAudioConverterRef);
								}

		static	OSStatus	fillBufferData(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
									AudioBufferList* ioBufferList,
									AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
								{
									// Setup
									Internals&	internals = *((Internals*) inUserData);

									// Check situation
									if (internals.mInputAudioFrames.hasInstance())
										// Reset
										internals.mInputAudioFrames->reset();
									else {
										// Setup
										CAudioFrames::Requirements	requirements =
																			internals.mAudioConverter
																					.queryRequirements();
										UInt32						frameCountInterval =
																			(requirements.mFrameCountInterval > 1) ?
																					requirements.mFrameCountInterval :
																					1024;
										UInt32						frameCount =
																			requirements.getFrameCount(
																					frameCountInterval * 10);
										if (internals.mInputAudioProcessingFormat->getIsInterleaved())
											// Interleaved
											internals.mInputAudioFrames =
													OI<CAudioFrames>(
															new CAudioFrames(
																	internals.mInputAudioProcessingFormat->
																			getBytesPerFrame(),
																	frameCount));
										else
											// Non-interleaved
											internals.mInputAudioFrames =
													OI<CAudioFrames>(
															new CAudioFrames(
																	internals.mInputAudioProcessingFormat->
																			getChannelMap().getChannelCount(),
																	internals.mInputAudioProcessingFormat->getBits() /
																			8,
																	frameCount));
									}

									// Check if have more to read
									OSStatus	status;
									if (internals.mSourceHasMoreToRead) {
										// Try to read
										TVResult<CAudioProcessor::SourceInfo>	audioProcessorSourceInfo =
																						internals.mAudioConverter.
																								CAudioProcessor::performInto(
																										*internals.mInputAudioFrames);
										if (audioProcessorSourceInfo.hasValue()) {
											// Success
											internals.mSourceTimeInterval = audioProcessorSourceInfo->getTimeInterval();
											status = noErr;
										} else if (audioProcessorSourceInfo.getError() == SError::mEndOfData) {
											// End of data
											internals.mSourceHasMoreToRead = false;
											status = noErr;
										} else {
											// Error
											internals.mFillBufferDataError = audioProcessorSourceInfo.getError();
											status = -1;
										}
									} else
										// No more source data, reset everything
										status = noErr;

									// Prepare return info
									*ioNumberDataPackets = internals.mInputAudioFrames->getAsRead(*ioBufferList);

									if (outDataPacketDescription != nil) *outDataPacketDescription = nil;

									return status;
								}

		CAudioConverter&				mAudioConverter;
		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;

		AudioBufferList*				mOutputAudioBufferList;
		AudioConverterRef				mAudioConverterRef;
		OI<CAudioFrames>				mInputAudioFrames;
		OV<SError>						mFillBufferDataError;
		bool							mSourceHasMoreToRead;
		UniversalTimeInterval			mSourceTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreAudioAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioAudioConverter::CCoreAudioAudioConverter() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioAudioConverter::~CCoreAudioAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCoreAudioAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup
	AudioStreamBasicDescription	sourceFormat;
	FillOutASBDForLPCM(sourceFormat, audioProcessingFormat.getSampleRate(),
			audioProcessingFormat.getChannelMap().getChannelCount(), audioProcessingFormat.getBits(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(), audioProcessingFormat.getIsBigEndian(),
			!audioProcessingFormat.getIsInterleaved());

	AudioStreamBasicDescription	destinationFormat;
	FillOutASBDForLPCM(destinationFormat, mOutputAudioProcessingFormat->getSampleRate(),
			mOutputAudioProcessingFormat->getChannelMap().getChannelCount(), mOutputAudioProcessingFormat->getBits(),
			mOutputAudioProcessingFormat->getBits(), mOutputAudioProcessingFormat->getIsFloat(),
			mOutputAudioProcessingFormat->getIsBigEndian(), !mOutputAudioProcessingFormat->getIsInterleaved());

	// Create Audio Converter
	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mInternals->mAudioConverterRef);
	LogOSStatusIfFailed(status, CString(OSSTR("AudioConverterNew")));

	if (audioProcessingFormat.getSampleRate() != mOutputAudioProcessingFormat->getSampleRate()) {
		// Highest quality SRC
		UInt32	value = kAudioCodecQuality_Max;
		status =
				::AudioConverterSetProperty(mInternals->mAudioConverterRef, kAudioConverterSampleRateConverterQuality,
						sizeof(UInt32), &value);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioConverterSetProperty")));
	}

	// Create Audio Buffer List
	if (mOutputAudioProcessingFormat->getIsInterleaved()) {
		// Interleaved
		mInternals->mOutputAudioBufferList = (AudioBufferList*) ::calloc(1, sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers = 1;
	} else {
		// Non-interleaved
		mInternals->mOutputAudioBufferList =
				(AudioBufferList*) ::calloc(mOutputAudioProcessingFormat->getChannelMap().getChannelCount(),
						sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers =
				mOutputAudioProcessingFormat->getChannelMap().getChannelCount();
	}

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CCoreAudioAudioConverter::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("CoreAudio Converter"));
	setupDescriptions +=
			indent + CString(OSSTR("    From: ")) + mInternals->mInputAudioProcessingFormat->getDescription();
	setupDescriptions += indent + CString(OSSTR("      To: ")) + mOutputAudioProcessingFormat->getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CCoreAudioAudioConverter::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	frameCount = audioFrames.getAsWrite(*mInternals->mOutputAudioBufferList);

	// Fill buffer
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef, Internals::fillBufferData,
								mInternals, &frameCount, mInternals->mOutputAudioBufferList, nil);
	if (status != noErr) return TVResult<SourceInfo>(*mInternals->mFillBufferDataError);
	if (frameCount == 0) return TVResult<SourceInfo>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(*mInternals->mOutputAudioBufferList);

	return TVResult<SourceInfo>(SourceInfo(mInternals->mSourceTimeInterval));
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreAudioAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mSourceHasMoreToRead = true;

	// Do super
	CAudioProcessor::reset();

	// Reset AudioConverter
	OSStatus	status = ::AudioConverterReset(mInternals->mAudioConverterRef);
	if (status != noErr) {
		// Log
		LogError(SErrorFromOSStatus(status), CString(OSSTR("calling AudioConverterReset")));

		return;
	}
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CCoreAudioAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getChannelMap()),
					SAudio::ProcessingSetup::kSampleTypeUnspecified, SAudio::ProcessingSetup::kEndianUnspecified,
					SAudio::ProcessingSetup::kInterleavedUnspecified));
}
