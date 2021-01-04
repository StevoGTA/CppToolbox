//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter-SecretRabbitCode.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

#include <samplerate.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#define SErrorFromSRC(error)			SError(CString(OSSTR("SRC")), error, CString(src_strerror(error)))
#define ReturnErrorIfSRCError(error)	if (error != 0) return OI<SError>(SErrorFromSRC(error));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioConverterInternals

class CAudioConverterInternals {
	public:
						CAudioConverterInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mSRCState(nil), mSourceHasMoreToRead(true),
										mSourceMediaPosition(SMediaPosition::fromStart(0.0)),
										mSourceSourceProcessed(0.0)
							{}
						~CAudioConverterInternals()
							{
								if (mSRCState != nil)
									src_delete(mSRCState);
							}

		static	long	fillBufferData(void* userData, float** data)
							{
								// Setup
								CAudioConverterInternals&	internals = *((CAudioConverterInternals*) userData);
								internals.mInputAudioData->reset();

								// Check if have more to read
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

										// Check if need to convert
										if (internals.mInputAudioProcessingFormat->getIsSignedInteger()) {
											// Convert
											internals.mInputFloatAudioData->reset();

											if (internals.mInputAudioProcessingFormat->getBits() == 16)
												// Convert SInt16 => Float32
												src_short_to_float_array(
														(short*) internals.mInputAudioData->getBytePtr(),
														(float*) internals.mInputFloatAudioData->getMutableBytePtr(),
														internals.mInputAudioData->getCurrentFrameCount() *
																internals.mInputAudioProcessingFormat->getChannels());
											else
												// Convert SInt32 => Float32
												src_int_to_float_array(
														(int*) internals.mInputAudioData->getBytePtr(),
														(float*) internals.mInputFloatAudioData->getMutableBytePtr(),
														internals.mInputAudioData->getCurrentFrameCount() *
																internals.mInputAudioProcessingFormat->getChannels());
										}
									} else if (*audioReadStatus.getError() == SError::mEndOfData) {
										// End of data
										internals.mSourceHasMoreToRead = false;
										internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
									} else
										// Error
										internals.mPerformError = audioReadStatus.getError();
								}

								// Prepare return info
								if (internals.mInputFloatAudioData.hasInstance())
									// Use converted float
									*data = (float*) internals.mInputFloatAudioData->getBytePtr();
								else
									// Use input float
									*data = (float*) internals.mInputAudioData->getBytePtr();

								return internals.mInputAudioData->getCurrentFrameCount();
							}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<SAudioProcessingFormat>	mOutputAudioProcessingFormat;

		SRC_STATE*					mSRCState;
		OI<CAudioData>				mInputAudioData;
		OI<CAudioData>				mInputFloatAudioData;
		OI<SError>					mPerformError;
		bool						mSourceHasMoreToRead;
		SMediaPosition				mSourceMediaPosition;
		Float32						mSourceSourceProcessed;
};

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
OI<SError> CAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Create Secret Rabbit Code
	int	error;
	mInternals->mSRCState =
		src_callback_new(CAudioConverterInternals::fillBufferData, SRC_SINC_BEST_QUALITY,
				audioProcessingFormat.getChannels(), &error, mInternals);
	ReturnErrorIfSRCError(error);

	// Create Audio Data(s)
	mInternals->mInputAudioData = OI<CAudioData>(new CAudioData(1, audioProcessingFormat.getBytesPerFrame()));
	if (audioProcessingFormat.getIsSignedInteger())
		// Need to convert input audio data to float
		mInternals->mInputFloatAudioData =
				OI<CAudioData>(new CAudioData(1, sizeof(Float32) * audioProcessingFormat.getChannels()));

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioConverter::perform(const SMediaPosition& mediaPosition, CAudioData& audioData)
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

	// Fill buffer
	double	srcRatio =
					mInternals->mOutputAudioProcessingFormat->getSampleRate() /
							mInternals->mInputAudioProcessingFormat->getSampleRate();
	frameCount =
			src_callback_read(mInternals->mSRCState, srcRatio, frameCount, (float*) audioData.getMutableBytePtr());
	if (frameCount == 0) return SAudioReadStatus(SError::mEndOfData);

	// Update
	audioData.completeWrite(frameCount);

	return SAudioReadStatus(mInternals->mSourceSourceProcessed);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	int	error = src_reset(mInternals->mSRCState);
	ReturnErrorIfSRCError(error);

	mInternals->mSourceHasMoreToRead = true;

	// Do super
	return CAudioProcessor::reset();
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup(*mInternals->mOutputAudioProcessingFormat));
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
