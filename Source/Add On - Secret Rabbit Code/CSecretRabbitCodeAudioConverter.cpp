//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter-SecretRabbitCode.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSecretRabbitCodeAudioConverter.h"

#include <samplerate.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#define SErrorFromSRC(error)			SError(CString(OSSTR("SRC")), error, CString(src_strerror(error)))
#define ReturnErrorIfSRCError(error)	if (error != 0) return OV<SError>(SErrorFromSRC(error));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSecretRabbitCodeAudioConverter::Internals

class CSecretRabbitCodeAudioConverter::Internals {
	public:
						Internals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mSRCState(nil), mSourceHasMoreToRead(true), mSourceTimeInterval(0.0)
							{}
						~Internals()
							{
								if (mSRCState != nil)
									src_delete(mSRCState);
							}

		static	long	fillBufferData(void* userData, float** data)
							{
								// Setup
								Internals&	internals = *((Internals*) userData);
								if (internals.mInputAudioFrames.hasInstance())
									// Reset
									internals.mInputAudioFrames->reset();
								else {
									// Setup
									CAudioFrames::Requirements	requirements =
																		internals.mAudioConverter.queryRequirements();
									UInt32						frameCountInterval =
																		(requirements.mFrameCountInterval > 1) ?
																				requirements.mFrameCountInterval : 1024;
									UInt32						frameCount =
																		requirements.getFrameCount(
																				frameCountInterval * 10);
									internals.mInputAudioFrames =
											OI<CAudioFrames>(
													new CAudioFrames(1,
															internals.mInputAudioProcessingFormat->getBytesPerFrame(),
															frameCount));
									if (internals.mInputAudioProcessingFormat->getIsSignedInteger())
										// Need to convert input audio frames to float
										internals.mInputFloatAudioFrames =
												OI<CAudioFrames>(
														new CAudioFrames(1,
																sizeof(Float32) *
																		internals.mInputAudioProcessingFormat->
																				getChannelMap().getChannelCount(),
																frameCount));
								}

								// Check if have more to read
								CAudioFrames::Info	readInfo = internals.mInputAudioFrames->getReadInfo();
								if (internals.mSourceHasMoreToRead) {
									// Try to read
									TVResult<CAudioProcessor::SourceInfo>	audioProcessorSourceInfo =
																					internals.mAudioConverter.CAudioProcessor::performInto(
																							*internals.mInputAudioFrames);
									if (audioProcessorSourceInfo.hasValue()) {
										// Success
										internals.mSourceTimeInterval = audioProcessorSourceInfo->getTimeInterval();

										// Check if need to convert
										if (internals.mInputAudioProcessingFormat->getIsSignedInteger()) {
											// Convert
											internals.mInputFloatAudioFrames->reset();

											CAudioFrames::Info	writeInfo =
																		internals.mInputFloatAudioFrames->
																				getWriteInfo();
											if (internals.mInputAudioProcessingFormat->getBits() == 16)
												// Convert SInt16 => Float32
												src_short_to_float_array((short*) readInfo.getSegments()[0],
														(float*) writeInfo.getSegments()[0],
														internals.mInputAudioFrames->getCurrentFrameCount() *
																internals.mInputAudioProcessingFormat->
																		getChannelMap().getChannelCount());
											else
												// Convert SInt32 => Float32
												src_int_to_float_array((int*) readInfo.getSegments()[0],
														(float*) writeInfo.getSegments()[0],
														internals.mInputAudioFrames->getCurrentFrameCount() *
																internals.mInputAudioProcessingFormat->
																		getChannelMap().getChannelCount());
										}
									} else if (audioProcessorSourceInfo.getError() == SError::mEndOfData) {
										// End of data
										internals.mSourceHasMoreToRead = false;
									} else
										// Error
										internals.mPerformError = audioProcessorSourceInfo.getError();
								}

								// Prepare return info
								if (internals.mInputFloatAudioFrames.hasInstance())
									// Use converted float
									*data = (float*) internals.mInputFloatAudioFrames->getWriteInfo().getSegments()[0];
								else
									// Use input float
									*data = (float*) readInfo.getSegments()[0];

								return internals.mInputAudioFrames->getCurrentFrameCount();
							}

		CAudioConverter&				mAudioConverter;
		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;

		SRC_STATE*						mSRCState;
		OI<CAudioFrames>				mInputAudioFrames;
		OI<CAudioFrames>				mInputFloatAudioFrames;
		OV<SError>						mPerformError;
		bool							mSourceHasMoreToRead;
		UniversalTimeInterval			mSourceTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSecretRabbitCodeAudioConverter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSecretRabbitCodeAudioConverter::CSecretRabbitCodeAudioConverter() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CSecretRabbitCodeAudioConverter::~CSecretRabbitCodeAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CSecretRabbitCodeAudioConverter::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Create Secret Rabbit Code
	int	error;
	mInternals->mSRCState =
			src_callback_new(Internals::fillBufferData, SRC_SINC_BEST_QUALITY,
					audioProcessingFormat.getChannelMap().getChannelCount(), &error, mInternals);
	ReturnErrorIfSRCError(error);

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CSecretRabbitCodeAudioConverter::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Secret Rabbit Code Audio Converter"));
	setupDescriptions +=
			indent + CString(OSSTR("    From: ")) + mInternals->mInputAudioProcessingFormat->getDescription();
	setupDescriptions += indent + CString(OSSTR("      To: ")) + mOutputAudioProcessingFormat->getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CSecretRabbitCodeAudioConverter::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Fill buffer
	double	srcRatio =
					mOutputAudioProcessingFormat->getSampleRate() /
							mInternals->mInputAudioProcessingFormat->getSampleRate();
	UInt32	frameCount =
					src_callback_read(mInternals->mSRCState, srcRatio, audioFrames.getAllocatedFrameCount(),
							(float*) audioFrames.getWriteInfo().getSegments()[0]);
	if (frameCount == 0) return TVResult<SourceInfo>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(frameCount);

	return TVResult<SourceInfo>(SourceInfo(mInternals->mSourceTimeInterval));
}

//----------------------------------------------------------------------------------------------------------------------
void CSecretRabbitCodeAudioConverter::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mSourceHasMoreToRead = true;

	// Do super
	CAudioProcessor::reset();

	// Reset
	src_reset(mInternals->mSRCState);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CSecretRabbitCodeAudioConverter::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getChannelMap()),
					SAudio::ProcessingSetup::kSampleTypeUnspecified, SAudio::ProcessingSetup::kEndianLittle,
					SAudio::ProcessingSetup::kInterleaved));
}
