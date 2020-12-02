//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

#include "CLogServices.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioConverterInternals

class CAudioConverterInternals {
	public:
		CAudioConverterInternals(CAudioConverter& audioConverter) :
			mAudioConverter(audioConverter),
					mOutputAudioBufferList(nil), mAudioConverterRef(nil),
					mSourceHasMoreToRead(true), mSourceMediaPosition(SMediaPosition::fromStart(0.0)),
							mSourceSourceProcessed(0.0)
			{}
		~CAudioConverterInternals()
			{
				::free(mOutputAudioBufferList);
				if (mAudioConverterRef != nil)
					::AudioConverterDispose(mAudioConverterRef);
			}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<SAudioProcessingFormat>	mOutputAudioProcessingFormat;

		AudioBufferList*			mOutputAudioBufferList;
		AudioConverterRef			mAudioConverterRef;
		OI<CAudioData>				mInputAudioData;
		bool						mSourceHasMoreToRead;
		SMediaPosition				mSourceMediaPosition;
		Float32						mSourceSourceProcessed;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	OSStatus	sFillBufferDataProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
							AudioBufferList* ioBufferList, AudioStreamPacketDescription** outDataPacketDescription,
							void* inUserData);


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioConverter::CAudioConverter() : CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioConverterInternals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioConverter::~CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup(*mInternals->mOutputAudioProcessingFormat));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
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
	FillOutASBDForLPCM(destinationFormat, mInternals->mOutputAudioProcessingFormat->getSampleRate(),
			mInternals->mOutputAudioProcessingFormat->getChannels(),
			mInternals->mOutputAudioProcessingFormat->getBits(), mInternals->mOutputAudioProcessingFormat->getBits(),
			mInternals->mOutputAudioProcessingFormat->getIsFloat(),
			mInternals->mOutputAudioProcessingFormat->getIsBigEndian(),
			!mInternals->mOutputAudioProcessingFormat->getIsInterleaved());

	// Create Audio Converter
	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mInternals->mAudioConverterRef);
	if (status != noErr)
		// Huh?
		CLogServices::logError(CString("CAudioConverter - error when creating AudioConverter: ") + CString(status));

	if (audioProcessingFormat.getSampleRate() != mInternals->mOutputAudioProcessingFormat->getSampleRate()) {
		// Highest quality SRC
		UInt32	value = kAudioCodecQuality_Max;
		status =
				::AudioConverterSetProperty(mInternals->mAudioConverterRef, kAudioConverterSampleRateConverterQuality,
						sizeof(UInt32), &value);
		if (status != noErr)
			// Huh?
			CLogServices::logError(CString("CAudioConverter - error when setting quality: ") + CString(status));
	}

	// Create Audio Buffer List
	if (mInternals->mOutputAudioProcessingFormat->getIsInterleaved()) {
		// Interleaved
		mInternals->mOutputAudioBufferList = (AudioBufferList*) ::calloc(1, sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers = 1;
	} else {
		// Non-interleaved
		mInternals->mOutputAudioBufferList =
				(AudioBufferList*) ::calloc(mInternals->mOutputAudioProcessingFormat->getChannels(),
						sizeof(AudioBufferList));
		mInternals->mOutputAudioBufferList->mNumberBuffers = mInternals->mOutputAudioProcessingFormat->getChannels();
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
TArray<SAudioProcessingSetup> CAudioConverter::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup::mUnspecified);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioConverter::setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mOutputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioConverter::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
// TODO: handle mediaPosition

	// Store
	mInternals->mSourceMediaPosition = mediaPosition;

	// Setup
	UInt32	packetCount = audioData.getAvailableFrameCount();
	audioData.getAsWrite(*mInternals->mOutputAudioBufferList);

	// Fill buffer
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef, sFillBufferDataProc,
								mInternals, &packetCount, mInternals->mOutputAudioBufferList, nil);
	if (status != noErr) return SAudioReadStatus(SError::fromOSStatus(status));
	if (packetCount == 0) return SAudioReadStatus(SError::mEndOfData);

	// Update
	audioData.completeWrite(packetCount);

	return SAudioReadStatus(mInternals->mSourceSourceProcessed);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	OSStatus	status = ::AudioConverterReset(mInternals->mAudioConverterRef);
	if (status != noErr) return SError::fromOSStatus(status);

	mInternals->mSourceHasMoreToRead = true;

	// Do super
	return CAudioProcessor::reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
OSStatus sFillBufferDataProc(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
		AudioBufferList* ioBufferList, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAudioConverterInternals&	internals = *((CAudioConverterInternals*) inUserData);
	internals.mInputAudioData->reset();

	// Check if have more to read
	OSStatus	status;
	if (internals.mSourceHasMoreToRead) {
		// Try to read
		SAudioReadStatus	audioReadStatus =
									internals.mAudioConverter.CAudioProcessor::perform(internals.mSourceMediaPosition,
											*internals.mInputAudioData);
		if (audioReadStatus.isSuccess()) {
			// Success
			internals.mSourceSourceProcessed = *audioReadStatus.getSourceProcessed();
			status = noErr;
		} else {
			// Check result
// For now we assume any error is just EOF.  When we switch over to SError globally, we can update this.

//			if (audioReadStatus.getError()->get
			// EOF error or other, reset return info
//			internals->mAudioDataArray->reset();

			// EOF is no error
//			if (error == kFileEOFError) {
				// No more source data
				internals.mSourceHasMoreToRead = false;

				status = noErr;
//			}
		}
	} else {
		// No more source data, reset everything
		status = noErr;
	}

	// Prepare return info
	internals.mInputAudioData->getAsRead(*ioBufferList);

	*ioNumberDataPackets = internals.mInputAudioData->getCurrentFrameCount();

	if (outDataPacketDescription != nil) *outDataPacketDescription = nil;

	return status;
}
