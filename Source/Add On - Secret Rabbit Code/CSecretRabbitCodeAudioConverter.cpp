//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter-SecretRabbitCode.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSecretRabbitCodeAudioConverter.h"

#include <samplerate.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#define SErrorFromSRC(error)			SError(CString(OSSTR("SRC")), error, CString(src_strerror(error)))
#define ReturnErrorIfSRCError(error)	if (error != 0) return OI<SError>(SErrorFromSRC(error));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSecretRabbitCodeAudioConverterInternals

class CSecretRabbitCodeAudioConverterInternals {
	public:
						CSecretRabbitCodeAudioConverterInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mSRCState(nil), mSourceHasMoreToRead(true),
										mSourceMediaPosition(SMediaPosition::fromStart(0.0)),
										mSourceSourceProcessed(0.0)
							{}
						~CSecretRabbitCodeAudioConverterInternals()
							{
								if (mSRCState != nil)
									src_delete(mSRCState);
							}

		static	long	fillBufferData(void* userData, float** data)
							{
								// Setup
								CSecretRabbitCodeAudioConverterInternals&	internals =
																					*((CSecretRabbitCodeAudioConverterInternals*)
																							userData);
								internals.mInputAudioFrames->reset();

								// Check if have more to read
								if (internals.mSourceHasMoreToRead) {
									// Try to read
									SAudioReadStatus	audioReadStatus =
																internals.mAudioConverter.CAudioProcessor::perform(
																		internals.mSourceMediaPosition,
																		*internals.mInputAudioFrames);
									if (audioReadStatus.isSuccess()) {
										// Success
										internals.mSourceMediaPosition = SMediaPosition::fromCurrent();
										internals.mSourceSourceProcessed = *audioReadStatus.getSourceProcessed();

										// Check if need to convert
										if (internals.mInputAudioProcessingFormat->getIsSignedInteger()) {
											// Convert
											internals.mInputFloatAudioFrames->reset();

											if (internals.mInputAudioProcessingFormat->getBits() == 16)
												// Convert SInt16 => Float32
												src_short_to_float_array(
														(short*) internals.mInputAudioFrames->getBytePtr(),
														(float*) internals.mInputFloatAudioFrames->getMutableBytePtr(),
														internals.mInputAudioFrames->getCurrentFrameCount() *
																internals.mInputAudioProcessingFormat->getChannels());
											else
												// Convert SInt32 => Float32
												src_int_to_float_array(
														(int*) internals.mInputAudioFrames->getBytePtr(),
														(float*) internals.mInputFloatAudioFrames->getMutableBytePtr(),
														internals.mInputAudioFrames->getCurrentFrameCount() *
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
								if (internals.mInputFloatAudioFrames.hasInstance())
									// Use converted float
									*data = (float*) internals.mInputFloatAudioFrames->getBytePtr();
								else
									// Use input float
									*data = (float*) internals.mInputAudioFrames->getBytePtr();

								return internals.mInputAudioFrames->getCurrentFrameCount();
							}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;

		SRC_STATE*					mSRCState;
		OI<CAudioFrames>			mInputAudioFrames;
		OI<CAudioFrames>			mInputFloatAudioFrames;
		OI<SError>					mPerformError;
		bool						mSourceHasMoreToRead;
		SMediaPosition				mSourceMediaPosition;
		Float32						mSourceSourceProcessed;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSecretRabbitCodeAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSecretRabbitCodeAudioConverter::CSecretRabbitCodeAudioConverter() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSecretRabbitCodeAudioConverterInternals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CSecretRabbitCodeAudioConverter::~CSecretRabbitCodeAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CSecretRabbitCodeAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Create Secret Rabbit Code
	int	error;
	mInternals->mSRCState =
		src_callback_new(CSecretRabbitCodeAudioConverterInternals::fillBufferData, SRC_SINC_BEST_QUALITY,
				audioProcessingFormat.getChannels(), &error, mInternals);
	ReturnErrorIfSRCError(error);

	// Create Audio Frames(s)
	mInternals->mInputAudioFrames = OI<CAudioFrames>(new CAudioFrames(1, audioProcessingFormat.getBytesPerFrame()));
	if (audioProcessingFormat.getIsSignedInteger())
		// Need to convert input audio frames to float
		mInternals->mInputFloatAudioFrames =
				OI<CAudioFrames>(new CAudioFrames(1, sizeof(Float32) * audioProcessingFormat.getChannels()));

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CSecretRabbitCodeAudioConverter::perform(const SMediaPosition& mediaPosition,
		CAudioFrames& audioFrames)
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

	UInt32	frameCount = audioFrames.getAvailableFrameCount();

	// Fill buffer
	double	srcRatio =
					mOutputAudioProcessingFormat->getSampleRate() /
							mInternals->mInputAudioProcessingFormat->getSampleRate();
	frameCount =
			src_callback_read(mInternals->mSRCState, srcRatio, frameCount, (float*) audioFrames.getMutableBytePtr());
	if (frameCount == 0) return SAudioReadStatus(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(frameCount);

	return SAudioReadStatus(mInternals->mSourceSourceProcessed);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CSecretRabbitCodeAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	int	error = src_reset(mInternals->mSRCState);
	ReturnErrorIfSRCError(error);

	mInternals->mSourceHasMoreToRead = true;

	// Do super
	return CAudioProcessor::reset();
}
