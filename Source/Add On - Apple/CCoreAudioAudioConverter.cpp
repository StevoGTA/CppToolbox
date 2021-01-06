//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioAudioConverter.h"

#include "CLogServices-Apple.h"
#include "SError-Apple.h"

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
										mSourceSourceProcessed(0.0)
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
									internals.mInputAudioData->reset();

									// Check if have more to read
									OSStatus	status;
									if (internals.mSourceHasMoreToRead) {
										// Try to read
										SAudioReadStatus	audioReadStatus =
																	internals.mAudioConverter.CAudioProcessor::perform(
																			internals.mSourceMediaPosition,
																			*internals.mInputAudioData);
										if (audioReadStatus.isSuccess()) {
											// Success
											internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
											internals.mSourceSourceProcessed = *audioReadStatus.getSourceProcessed();
											status = noErr;
										} else if (*audioReadStatus.getError() == SError::mEndOfData) {
											// End of data
											internals.mSourceHasMoreToRead = false;
											internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
											status = noErr;
										} else {
											// Error
											internals.mPerformError = audioReadStatus.getError();
											status = -1;
										}
									} else
										// No more source data, reset everything
										status = noErr;

									// Prepare return info
									internals.mInputAudioData->getAsRead(*ioBufferList);

									*ioNumberDataPackets = internals.mInputAudioData->getCurrentFrameCount();

									if (outDataPacketDescription != nil) *outDataPacketDescription = nil;

									return status;
								}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;

		AudioBufferList*			mOutputAudioBufferList;
		AudioConverterRef			mAudioConverterRef;
		OI<CAudioData>				mInputAudioData;
		OI<SError>					mPerformError;
		bool						mSourceHasMoreToRead;
		SMediaPosition				mSourceMediaPosition;
		Float32						mSourceSourceProcessed;
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
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioConverterNew"));

	if (audioProcessingFormat.getSampleRate() != mOutputAudioProcessingFormat->getSampleRate()) {
		// Highest quality SRC
		UInt32	value = kAudioCodecQuality_Max;
		status =
				::AudioConverterSetProperty(mInternals->mAudioConverterRef, kAudioConverterSampleRateConverterQuality,
						sizeof(UInt32), &value);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioConverterSetProperty"));
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
		mInternals->mInputAudioData = OI<CAudioData>(new CAudioData(1, audioProcessingFormat.getBytesPerFrame()));
	else
		// Non-interleaved
		mInternals->mInputAudioData =
				OI<CAudioData>(
						new CAudioData(audioProcessingFormat.getChannels(), audioProcessingFormat.getBits() / 8));

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CCoreAudioAudioConverter::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mSourceMediaPosition = mediaPosition;

	// Setup
	if ((mediaPosition.getMode() != SMediaPosition::kFromCurrent) && (mInternals->mSourceSourceProcessed != 0.0)) {
		// Reset
		OI<SError>	error = reset();
		if (error.hasInstance())
			// Error
			return SAudioReadStatus(*error);
	}

	UInt32	frameCount = audioData.getAvailableFrameCount();
	audioData.getAsWrite(*mInternals->mOutputAudioBufferList);

	// Fill buffer
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
								CCoreAudioAudioConverterInternals::fillBufferData, mInternals, &frameCount,
								mInternals->mOutputAudioBufferList, nil);
	if (status != noErr) return SAudioReadStatus(*mInternals->mPerformError);
	if (frameCount == 0) return SAudioReadStatus(SError::mEndOfData);

	// Update
	audioData.completeWrite(frameCount);

	return SAudioReadStatus(mInternals->mSourceSourceProcessed);
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
