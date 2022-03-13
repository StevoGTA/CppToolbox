//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationResampler.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationResampler.h"

#include "CMediaFoundationServices.h"

#undef Delete

#include <mfapi.h>
#include <mftransform.h>

#define Delete(x)	{ delete x; x = nil; }

#pragma comment( lib, "wmcodecdspuuid" )

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationResampler"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationResamplerInternals

class CMediaFoundationResamplerInternals {
	public:
							CMediaFoundationResamplerInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mSourceTimeInterval(0.0)
								{}
							~CMediaFoundationResamplerInternals()
								{
									// Cleanup
									MFShutdown();
								}

		static	OI<SError>	fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									CMediaFoundationResamplerInternals&	internals =
																				*((CMediaFoundationResamplerInternals*)
																						userData);

									// Load
									SAudioSourceStatus	audioSourceStatus =
																CMediaFoundationServices::load(mediaBuffer,
																		internals.mAudioConverter,
																		internals.mInputAudioProcessingFormat->
																				getBytesPerFrame(),
																		internals.mInputAudioProcessingFormat->getBits()
																				== 8);
									if (!audioSourceStatus.isSuccess())
										return OI<SError>(audioSourceStatus.getError());

									// Store
									internals.mSourceTimeInterval = audioSourceStatus.getTimeInterval();

									return OI<SError>();
								}

		CAudioConverter&			mAudioConverter;
		OI<SAudioProcessingFormat>	mInputAudioProcessingFormat;

		OCI<IMFTransform>			mResamplerTransform;

		OCI<IMFSample>				mInputSample;
		UniversalTimeInterval		mSourceTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationResampler

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationResampler::CMediaFoundationResampler() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMediaFoundationResamplerInternals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationResampler::~CMediaFoundationResampler()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationResampler::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Startup Media Foundation
	HRESULT	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	ReturnErrorIfFailed(result, OSSTR("MFStartup()"));

	// Store
	mInternals->mInputAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup transform
	TCIResult<IMFTransform>	resamplerTransform =
									CMediaFoundationServices::createTransformForAudioResampler(audioProcessingFormat,
											*mOutputAudioProcessingFormat);
	if (resamplerTransform.hasError())
		return OI<SError>(resamplerTransform.getError());
	mInternals->mResamplerTransform = resamplerTransform.getInstance();

	// Begin streaming!
	result = mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CMediaFoundationResampler::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mInternals->mResamplerTransform.hasInstance())
		// Failed initialization
		return SAudioSourceStatus(sSetupDidNotCompleteError);

	// Check if need to create input sample
	if (!mInternals->mInputSample.hasInstance()) {
		// Create input sample
		CAudioProcessor::Requirements	audioProcessorRequirements = queryRequirements();
		UInt32							byteCount =
												audioProcessorRequirements.mAudioFramesRequirements.getFrameCount(1024)
														* mInternals->mInputAudioProcessingFormat->getBytesPerFrame();
		TCIResult<IMFSample>			sample = CMediaFoundationServices::createSample(byteCount);
		ReturnValueIfResultError(sample, SAudioSourceStatus(sample.getError()));
		mInternals->mInputSample = sample.getInstance();
	}

	// Fill audio frames as much as we can
	while (audioFrames.getCurrentFrameCount() < audioFrames.getAvailableFrameCount()) {
		// Create output sample
		TCIResult<IMFSample>	sample =
										CMediaFoundationServices::createSample(
												audioFrames.getAvailableFrameCount() *
														mOutputAudioProcessingFormat->getBytesPerFrame());
		ReturnValueIfResultError(sample, SAudioSourceStatus(sample.getError()));

		// Process output
		OI<SError>	error =
							CMediaFoundationServices::processOutput(*mInternals->mResamplerTransform,
									*sample.getInstance(),
									CMediaFoundationServices::ProcessOutputInfo(
											CMediaFoundationResamplerInternals::fillInputBuffer,
											mInternals->mInputSample, mInternals));
		ReturnValueIfError(error, SAudioSourceStatus(*error));

		// Complete write
		error =
				CMediaFoundationServices::completeWrite(*sample.getInstance(), 0, audioFrames,
						*mOutputAudioProcessingFormat);
		ReturnValueIfError(error, SAudioSourceStatus(*error));
	}

	return SAudioSourceStatus(mInternals->mSourceTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaFoundationResampler::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CAudioProcessor::reset();

	// Reset Resampler
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
}
