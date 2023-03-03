//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioAudioConverter.h"

#include "SError-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioAudioConverterInternals

class CCoreAudioAudioConverterInternals {
	public:
							CCoreAudioAudioConverterInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mOutputAudioBufferList(nil), mAudioConverterRef(nil),
										mSourceHasMoreToRead(true), mSourceTimeInterval(0.0)
								{}
							~CCoreAudioAudioConverterInternals()
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
									CCoreAudioAudioConverterInternals&	internals =
																				*((CCoreAudioAudioConverterInternals*)
																						inUserData);

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
															new CAudioFrames(1,
																	internals.mInputAudioProcessingFormat->
																			getBytesPerFrame(),
																	frameCount));
										else
											// Non-interleaved
											internals.mInputAudioFrames =
													OI<CAudioFrames>(
															new CAudioFrames(
																	internals.mInputAudioProcessingFormat->
																			getChannels(),
																	internals.mInputAudioProcessingFormat->getBits() /
																			8,
																	frameCount));
									}

									// Check if have more to read
									OSStatus	status;
									if (internals.mSourceHasMoreToRead) {
										// Try to read
										SAudioSourceStatus	audioSourceStatus =
																	internals.mAudioConverter.
																			CAudioProcessor::performInto(
																					*internals.mInputAudioFrames);
										if (audioSourceStatus.isSuccess()) {
											// Success
											internals.mSourceTimeInterval = audioSourceStatus.getTimeInterval();
											status = noErr;
										} else if (audioSourceStatus.getError() == SError::mEndOfData) {
											// End of data
											internals.mSourceHasMoreToRead = false;
											status = noErr;
										} else {
											// Error
											internals.mFillBufferDataError = audioSourceStatus.getError();
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

		CAudioConverter&			mAudioConverter;
		OV<SAudioProcessingFormat>	mInputAudioProcessingFormat;

		AudioBufferList*			mOutputAudioBufferList;
		AudioConverterRef			mAudioConverterRef;
		OI<CAudioFrames>			mInputAudioFrames;
		OV<SError>					mFillBufferDataError;
		bool						mSourceHasMoreToRead;
		UniversalTimeInterval		mSourceTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreAudioAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioAudioConverter::CCoreAudioAudioConverter() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CCoreAudioAudioConverterInternals(*this);
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
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OV<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup
	AudioStreamBasicDescription	sourceFormat;
	FillOutASBDForLPCM(sourceFormat, audioProcessingFormat.getSampleRate(), audioProcessingFormat.getChannels(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(),
			audioProcessingFormat.getIsBigEndian(), !audioProcessingFormat.getIsInterleaved());

	AudioStreamBasicDescription	destinationFormat;
	FillOutASBDForLPCM(destinationFormat, mOutputAudioProcessingFormat->getSampleRate(),
			mOutputAudioProcessingFormat->getChannels(), mOutputAudioProcessingFormat->getBits(),
			mOutputAudioProcessingFormat->getBits(), mOutputAudioProcessingFormat->getIsFloat(),
			mOutputAudioProcessingFormat->getIsBigEndian(), !mOutputAudioProcessingFormat->getIsInterleaved());

	// Create Audio Converter
	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mInternals->mAudioConverterRef);
	LogOSStatusIfFailed(status, OSSTR("AudioConverterNew"));

	if (audioProcessingFormat.getSampleRate() != mOutputAudioProcessingFormat->getSampleRate()) {
		// Highest quality SRC
		UInt32	value = kAudioCodecQuality_Max;
		status =
				::AudioConverterSetProperty(mInternals->mAudioConverterRef, kAudioConverterSampleRateConverterQuality,
						sizeof(UInt32), &value);
		LogOSStatusIfFailed(status, OSSTR("AudioConverterSetProperty"));
	}

	// Create Audio Buffer List
	if (mOutputAudioProcessingFormat->getIsInterleaved()) {
		// Interleaved
		mInternals->mOutputAudioBufferList = (AudioBufferList*) ::calloc(1, sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers = 1;
	} else {
		// Non-interleaved
		mInternals->mOutputAudioBufferList =
				(AudioBufferList*) ::calloc(mOutputAudioProcessingFormat->getChannels(), sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers = mOutputAudioProcessingFormat->getChannels();
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
SAudioSourceStatus CCoreAudioAudioConverter::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	frameCount = audioFrames.getAsWrite(*mInternals->mOutputAudioBufferList);

	// Fill buffer
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
								CCoreAudioAudioConverterInternals::fillBufferData, mInternals, &frameCount,
								mInternals->mOutputAudioBufferList, nil);
	if (status != noErr) return SAudioSourceStatus(*mInternals->mFillBufferDataError);
	if (frameCount == 0) return SAudioSourceStatus(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(*mInternals->mOutputAudioBufferList);

	return SAudioSourceStatus(mInternals->mSourceTimeInterval);
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
		LogError(SErrorFromOSStatus(status), "calling AudioConverterReset");

		return;
	}
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CCoreAudioAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(SAudioProcessingSetup::BitsInfo::mUnspecified,
					SAudioProcessingSetup::SampleRateInfo::mUnspecified,
					SAudioProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getAudioChannelMap()),
					SAudioProcessingSetup::kSampleTypeUnspecified, SAudioProcessingSetup::kEndianUnspecified,
					SAudioProcessingSetup::kInterleavedUnspecified));
}
