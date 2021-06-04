//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioAudioConverter.h"

#include "CLogServices-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioAudioConverterInternals

class CCoreAudioAudioConverterInternals {
	public:
							CCoreAudioAudioConverterInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mOutputAudioBufferList(nil), mAudioConverterRef(nil),
										mSourceHasMoreToRead(true),
										mSourceMediaPosition(SMediaPosition::fromStart(0.0)),
										mSourcePercentConsumed(0.0)
								{}
							~CCoreAudioAudioConverterInternals()
								{
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
									internals.mInputAudioFrames->reset();

									// Check if have more to read
									OSStatus	status;
									if (internals.mSourceHasMoreToRead) {
										// Try to read
										SAudioSourceStatus	audioSourceStatus =
																	internals.mAudioConverter.CAudioProcessor::perform(
																			internals.mSourceMediaPosition,
																			*internals.mInputAudioFrames);
										if (audioSourceStatus.isSuccess()) {
											// Success
											internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
											internals.mSourcePercentConsumed = *audioSourceStatus.getPercentConsumed();
											status = noErr;
										} else if (*audioSourceStatus.getError() == SError::mEndOfData) {
											// End of data
											internals.mSourceHasMoreToRead = false;
											internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
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
									internals.mInputAudioFrames->getAsRead(*ioBufferList);

									*ioNumberDataPackets = internals.mInputAudioFrames->getCurrentFrameCount();

									if (outDataPacketDescription != nil) *outDataPacketDescription = nil;

									return status;
								}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;

		AudioBufferList*			mOutputAudioBufferList;
		AudioConverterRef			mAudioConverterRef;
		OI<CAudioFrames>			mInputAudioFrames;
		OI<SError>					mFillBufferDataError;
		bool						mSourceHasMoreToRead;
		SMediaPosition				mSourceMediaPosition;
		Float32						mSourcePercentConsumed;
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
OI<SError> CCoreAudioAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

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

	// Create Audio Data
	if (audioProcessingFormat.getIsInterleaved())
		// Interleaved
		mInternals->mInputAudioFrames = OI<CAudioFrames>(new CAudioFrames(1, audioProcessingFormat.getBytesPerFrame()));
	else
		// Non-interleaved
		mInternals->mInputAudioFrames =
				OI<CAudioFrames>(
						new CAudioFrames(audioProcessingFormat.getChannels(), audioProcessingFormat.getBits() / 8));

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CCoreAudioAudioConverter::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mSourceMediaPosition = mediaPosition;

	// Setup
	if ((mediaPosition.getMode() != SMediaPosition::kFromCurrent) && (mInternals->mSourcePercentConsumed != 0.0)) {
		// Reset
		OI<SError>	error = reset();
		if (error.hasInstance())
			// Error
			return SAudioSourceStatus(*error);
	}

	UInt32	frameCount = audioFrames.getAvailableFrameCount();
	audioFrames.getAsWrite(*mInternals->mOutputAudioBufferList);

	// Fill buffer
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
								CCoreAudioAudioConverterInternals::fillBufferData, mInternals, &frameCount,
								mInternals->mOutputAudioBufferList, nil);
	if (status != noErr) return SAudioSourceStatus(*mInternals->mFillBufferDataError);
	if (frameCount == 0) return SAudioSourceStatus(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(frameCount);

	return SAudioSourceStatus(mInternals->mSourcePercentConsumed);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CCoreAudioAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	OSStatus	status = ::AudioConverterReset(mInternals->mAudioConverterRef);
	if (status != noErr) return SErrorFromOSStatus(status);

	mInternals->mSourceHasMoreToRead = true;

	// Do super
	return CAudioProcessor::reset();
}
